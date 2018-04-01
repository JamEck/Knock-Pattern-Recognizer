// James Eckdahl, Chad Pennington
// Knock Pattern Recognizer
int meas = 0;
const int siz = 10;
int buf[siz];
int counter = 0;
float avg = 0;
int avglvl = 0;
int measPin = A10;
//bool frameTap = 0;
bool waiting = 0;
bool result = 0;
float tol = 0.2;

int secs = 8;

int greenLED = 13;
int redLED   = 12;

int next = 0;

float* trainArr;
int trainSiz = 0;
float* testArr;

//////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  pinMode(measPin, INPUT);
  pinMode(13, OUTPUT);
  initArray(buf, siz, analogRead(measPin));

  avglvl = calibration();
  Serial.print("Calibration Level: ");
  Serial.println(avglvl);

  Serial.print("Now Recording for ");
  Serial.print(secs);
  Serial.println(" seconds: ");

  trainSiz = train(); //populate trainArray
  Serial.print("Pattern Length: ");
  Serial.println(trainSiz);
  
  tol = (trainSiz/5+1)*0.2;
  Serial.print("Error Tolerance: ");
  Serial.println(tol);

  
  getRatios(trainArr, trainSiz);

  Serial.print("Trained Pattern: ");
  for(int i = 0; i < trainSiz; i++){
  Serial.print(trainArr[i]);
  Serial.print(" ");
  }
  Serial.print("\n\n");

  testArr = (float*)malloc(sizeof(float)*trainSiz);

  Serial.println("Ready to record test knock.\nTap once to begin.\n");
}

//////////////////////////////////////////////////////

void loop() {

  meas = analogRead(measPin);
  buf[counter] = meas;
  counter++;
  if (counter >= siz)
    counter = 0;

  avg = averager(buf, siz);

  if(framePress(avg, avglvl, &waiting)){
    test();

  getRatios(testArr, trainSiz);
  
  Serial.print("Trained  Pattern: ");
  for(int i = 0; i < trainSiz; i++){
    Serial.print(trainArr[i]);
    Serial.print(" ");
  }
  Serial.print("\n");

  Serial.print("Inputted Pattern: ");
  for(int i = 0; i < trainSiz; i++){
    Serial.print(testArr[i]);
    Serial.print(" ");
  }
  Serial.print("\n\n");
  
  result = compareBeat(testArr, trainArr);

  Serial.println("Tap once to record again.\n");
  }
  delay(5);
}

//////////////////////////////////////////////////////

void initArray(int* buf_, int siz_, int val_){
  for(int i = 0; i < siz_; i++){ // initialize array
    buf_[i] = val_;
  }
}

void initArray(float* buf_, int siz_, int val_){
  for(int i = 0; i < siz_; i++){ // initialize array
    buf_[i] = (float)val_;
  }
}

//////////////////////////////////////////////////////

int calibration(){
  int soundlvl_ = 0;
  int meas_ = 0;
  for (int i = 0; i < 100; i++){
    soundlvl_ += analogRead(measPin);
      delay(10);
  }
  soundlvl_ /= 100;

  return soundlvl_;
}

//////////////////////////////////////////////////////

bool framePress(float avg_, float avglvl_, bool* waiting_){
  bool frameTap = 0;
  if (avg_ > avglvl_*1.1){
    if(*waiting_){
      frameTap = 1;
      *waiting_ = 0;
    }
  }
  else
    *waiting_ = 1;

    return frameTap;
}

//////////////////////////////////////////////////////

float averager(int* arr_, int siz_){
  float avg_ = 0;
  for(int i = 0; i < siz_; i++)
    avg_ += arr_[i];
  avg_ /= (float)siz_;

  return avg_;
}

//////////////////////////////////////////////////////

void getRatios(float* theArray_, int siz_){   
   for (int i = 0; i < siz_-1; i++){
        theArray_[i] = theArray_[i+1] - theArray_[i];
   }
  
   for (int i = 0; i < siz_-2; i++){
          theArray_[i] = (float)theArray_[i+1]/(float)theArray_[i];
   }

   theArray_[siz_-1] = 0;
   theArray_[siz_-2] = 0;
}

//////////////////////////////////////////////////////

int train(){
const int siz1 = 25;
float tempArr[siz1];
initArray(tempArr, siz1, 0);

bool go = 1;
long start = millis();
  while((millis() - start < secs*1000) && go){
  meas = analogRead(measPin);
  buf[counter] = meas;
  counter++;
  if (counter >= siz)
    counter = 0;

  avg = averager(buf, siz);

  if (framePress(avg, avglvl, &waiting)){ //tap of sensor
    Serial.print(next+1);
    Serial.print(" ");
    tempArr[next] = millis(); // get time stamps
    next++;
    if (next >= siz1){
      go = 0;
    }
  }
    delay(5);
  }Serial.println("\n");
  
  int i = 0;
  while(tempArr[i])i++;

  trainArr = (float*)malloc(sizeof(float)*i);
  for (int j = 0; j < i; j++){
    trainArr[j] = tempArr[j];
  }
  next = 0;
return i;
}

//////////////////////////////////////////////////////

void test(){
  initArray(testArr, trainSiz, 0);
  bool go = 1;
  Serial.println("Now Recording! Enter Pattern.\n");
  while(go){
  meas = analogRead(measPin);
  buf[counter] = meas;
  counter++;
  if (counter >= siz)
    counter = 0;

  avg = averager(buf, siz);

  if (framePress(avg, avglvl, &waiting)){ //tap of sensor
    Serial.print(next+1);
    Serial.print(" ");
    testArr[next] = millis(); // get time stamps
    next++;
    if (next >= trainSiz){
      go = 0;
    }
  }
    delay(5);
  }Serial.println("\n");
  next = 0;
}

//////////////////////////////////////////////////////

bool compareBeat(float* testarray, float* trainarray){
    float compArr = 0;
    float square = 0;
    float dist = 0;
    
    Serial.print("Square Error of Each Beat: ");
    for(int i = 0; i < trainSiz; i++){
      compArr = trainarray[i] - testarray[i];
      square = power(compArr, 2);
      Serial.print(square);
      Serial.print(" ");
      dist += square;
    }Serial.print("\n");
    Serial.print("Square Error Sum:       ");
    Serial.println(dist);
    Serial.print("Square Error Tolerance: ");
    Serial.println(tol);
    
    if (dist > tol)
    {
      Serial.println("DENIED: Red LED\n");
      digitalWrite(redLED, HIGH);
      delay(250);
      digitalWrite(redLED, LOW);      
      return false;
    }
    if (dist <= tol)
    {
      Serial.println("PASSED: Green LED\n");
      digitalWrite(greenLED, HIGH);
      delay(250);
      digitalWrite(greenLED, LOW);
      return true;
    }

}

//////////////////////////////////////////////////////

float power(float base, int ex){
    float ans = 1.0;
    for(int i = 0; i < ex; i++)
    ans*= base;

    return ans;
}
