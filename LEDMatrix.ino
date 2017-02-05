//assign pins to row/column
const int row1 = 6;
const int row2 = 7;
const int row3 = 8;
const int row4 = 9;
const int row5 = 10;
const int row6 = 11;
const int row7 = 12;
const int row8 = 13;
const int col1 = 5;
const int col2 = 4;
const int col3 = 3;
const int col4 = 16;
const int col5 = 17;
const int col6 = 18;
const int col7 = 19;

//number of rows/columns
const int rows = 8;
const int cols = 7;

//assign pin variables to array
//store the current state of col/row to array
int row[rows] = {row1,row2,row3,row4,row5,row6,row7,row8};
int col[cols] = {col1,col2,col3,col4,col5,col6,col7};

//2D array to store the state of each LED
bool state[rows][cols] = {
{LOW,LOW,LOW,LOW,LOW,LOW,LOW},
{LOW,LOW,LOW,LOW,LOW,LOW,LOW},
{LOW,LOW,LOW,LOW,LOW,LOW,LOW},
{LOW,LOW,LOW,LOW,LOW,LOW,LOW},
{LOW,LOW,LOW,LOW,LOW,LOW,LOW},
{LOW,LOW,LOW,LOW,LOW,LOW,LOW},
{LOW,LOW,LOW,LOW,LOW,LOW,LOW},
{LOW,LOW,LOW,LOW,LOW,LOW,LOW}
};

//variables for interrupt
volatile bool buttonAction = LOW;
volatile bool buttonReset = LOW;
volatile int buttonVoltage = 0;

//variables for scrolling function
int scrollingStep = 1; //current step
int currentCol = 0;
int currentRow = 0;
int scrollNumber = 3; //number of LEDs to scroll across the row
unsigned long timerScrolling = 0;
int intervalScrolling = 200;

//variables for row detect funtion
int detectStep = 1;
unsigned long timerDetect = 0;
int intervalDetect = 1000;
bool gameover = false;

//variables for gameOver function
unsigned long timerGameOver = 0;
int intervalGameOver = 500;
int gameNumber = 0;

//variables for calcuclateScore funtion
int currentScore = 0;
int highScore = 0;
unsigned long currentTime = 0;
float highScoreTime = 0;
float seconds = 0;

//variables for timer function
volatile unsigned long milliseconds = 0;





//interrupt occurs every 1.024 milliseconds
//for every interrupt, milliseconds increases by approximately 1
ISR(TIMER2_OVF_vect)
{
  milliseconds++;
}





void updateLED()
{
  //updates LED matrix by cycling through the row/col states
  //rapidly sets and resets the LEDs so that in reality only one LED is ever lit at one time  
  for(int i = 0; i < rows; i++)
  {
    for(int j = 0; j < cols; j++)
    {
      if(state[i][j])
      {
        digitalWrite(row[i],LOW);
        digitalWrite(col[j],HIGH);
        digitalWrite(row[i],HIGH);
        digitalWrite(col[j],LOW);
      }
    }
  }
}




void buttonDetect()
{
  buttonVoltage = analogRead(A0);
  if(buttonVoltage > 915 && buttonVoltage < 945)
  {
    buttonAction = HIGH;
  }
  else if(buttonVoltage > 1015)
  {
    buttonReset = HIGH;
  } 
}




void scrolling()
{
  //scrolls for current row until interrupt
  //timer to delay col change
  switch(scrollingStep)
  {
    case 1: //initialize scrolling
      timerScrolling = milliseconds;
      state[currentRow][0] = HIGH;
      if(scrollNumber == 1)
      {
        currentCol = 1;
      }
      else if(scrollNumber == 2)
      {
        state[currentRow][1] = HIGH;
        currentCol = 2;
      }
      else
      {
        state[currentRow][1] = HIGH;
        state[currentRow][2] = HIGH;
        currentCol = 3;
      }
      scrollingStep++;      
      break;  
    case 2: //increasing
      if (currentCol < cols)
      {
        if(milliseconds - timerScrolling >= intervalScrolling)
        {
          state[currentRow][currentCol] = HIGH;
          
          if(scrollNumber == 1)
            state[currentRow][currentCol-1] = LOW;
          else if(scrollNumber == 2)
            state[currentRow][currentCol-2] = LOW;
          else
            state[currentRow][currentCol-3] = LOW;
            
          currentCol++;
          timerScrolling = milliseconds;
        }
      }
      else
      {
        if(scrollNumber == 1)
          currentCol = cols-2;
        else if(scrollNumber == 2)
          currentCol = cols-3;
        else
          currentCol = cols-4;
        scrollingStep++;
      }
      break;      
    case 3: //decreasing
      if(currentCol >= 0)
      {
        if(milliseconds - timerScrolling >= intervalScrolling)
        {
          state[currentRow][currentCol] = HIGH;
          
           if(scrollNumber == 1)
            state[currentRow][currentCol+1] = LOW;
          else if(scrollNumber == 2)
            state[currentRow][currentCol+2] = LOW;
          else
            state[currentRow][currentCol+3] = LOW;
            
          currentCol--;
          timerScrolling = milliseconds;
        }
      }
      else
      {
        if(scrollNumber == 1)
          currentCol = 1;
        else if(scrollNumber == 2)
          currentCol = 2;
        else
          currentCol = 3;
        scrollingStep--;
      }
      break;
    default:
      break;
  }
}






void detectRow()
{
  switch(detectStep)
  {
    case 1:
      timerDetect = milliseconds;
      detectStep++;
      break;
    case 2:
      if(milliseconds - timerDetect >= intervalDetect) //waits for a period for a period of time before actually detecting hit/miss
      {
        if(currentRow != 0)
        {
          for(int i = 0; i < cols; i++)
          {
            if(state[currentRow][i]) //first look for the HIGH LEDs, then look below
            {
              if(!state[currentRow-1][i])//if below LED is LOW, then the player has missed, and the upper LED should be low too
              {
                state[currentRow][i] = LOW;
                scrollNumber--; //decrease the number of LEDs to scroll on the next row
              }
            }
          }
        }
        
        
        if(scrollNumber <= 0) //if the number of LEDs goes to 0, then trigger the gameover function
        {
          gameover = true;
          Serial.println("GAME OVER");
        }
        else if(currentRow == 7)
        {
          gameover = true;
          currentTime = milliseconds;
          currentRow++;
          Serial.println("CONGRATULATIONS, YOU WIN!");
        }
        else
        {
          currentTime = milliseconds;
          currentRow++;
          intervalScrolling *=0.8;
          scrollingStep = 1;
          buttonAction = LOW;
          detectStep--;
          }
      }
      break;
    default:
      break;
  }
}






void reset()
{
  for(int i = 0; i < rows; i++)
  {
    for(int j = 0; j < cols; j++)
    {
      state[i][j] = LOW;
    }
  }

  buttonAction = LOW;
  buttonReset = LOW;
  
  scrollingStep = 1;
  currentCol = 0;
  currentRow = 0;
  scrollNumber = 3;
  intervalScrolling = 200;
  
  detectStep = 1;
  gameover = false;

  milliseconds = 0;
}







void gameOver()
{
  timerGameOver = milliseconds;
  gameNumber++;
  Serial.print("Game ");
  Serial.println(gameNumber);
  calculateScore();
  
  while(gameover && !buttonReset)
  {
    if(milliseconds - timerGameOver < intervalGameOver)
      ;
    else if(milliseconds- timerGameOver < (intervalGameOver*2))
      updateLED();
    else
      timerGameOver = milliseconds;      
  }
}






void calculateScore()
{
  currentScore = currentRow;

  seconds = (float)currentTime/1000;

  Serial.print("Your score is: ");
  Serial.println(currentScore);
  Serial.print("Time taken was: ");
  Serial.print(seconds,2);
  Serial.println(" seconds");

  if(currentScore > highScore)
  {
    highScore = currentScore;
    highScoreTime = seconds;
    Serial.println("Congratulations, new high score!");
  }
  else if (currentScore == highScore && seconds < highScoreTime)
  {
    highScoreTime = seconds;
    Serial.println("Congratulations, new high score!");
  }
  else
  {
    Serial.print("The high score is: ");
    Serial.print(highScore);
    Serial.print(" with a time of ");
    Serial.print (highScoreTime,2);
    Serial.println(" seconds");
  }

  Serial.println();
}







void setup()
{
  Serial.begin(9600);

  //sets prescaler to CLK/64
  bitClear(TCCR2B,CS22);
  bitSet(TCCR2B,CS21);
  bitSet(TCCR2B,CS20);
  //enables overflow interrupt
  bitSet(TIMSK2,TOIE2);
  //enables overflow flag
  bitSet(TIFR2,TOV2);
  
  attachInterrupt(digitalPinToInterrupt(2),buttonDetect,RISING);

  for(int i = 0; i < rows; i++)
  {
    pinMode(row[i],OUTPUT);
    digitalWrite(row[i],HIGH);
  }
  for(int i = 0; i < cols; i++)
    pinMode(col[i],OUTPUT);  
}






void loop()
{
  //scrolls the current LED row back and forth until the action button is pressed
  if(!buttonAction)
    scrolling();
  else if(!gameover)
    detectRow();
  else
    gameOver();

  //resets game when reset button is pressed
  if(buttonReset)
    reset();
  
    
  //constantly updates the LED matrix
  updateLED();
}
