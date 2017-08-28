#include <SIM900.h>

Connection * SIM900 = NULL;

unsigned int  httpCodeResponse;
char * bodyResponse = NULL;

void setup()
{ 
  // Init SIM900 library ( PinCode, APN, APN_USER, APN_PASSWORD, ENABLE_PIN, SERIAL )
  SIM900 = new Connection( "1345", "internet", "", "", 2, Serial );
  
  // Configure SIM900 library
  if ( !SIM900->Configuration() )
  {
    Serial.println( "@@@@@@@@@@@@@@@@@@@@@@@@@@" );
    Serial.println( "SIM900 CONFIGURATION FAIL" );
    Serial.println( "@@@@@@@@@@@@@@@@@@@@@@@@@@" );
  }
}


void loop()
{  
  
  // Make a GET petition to the server
  if ( SIM900->Get( "private-b86c1-sim900test.apiary-mock.com", "api", "notes/2", httpCodeResponse, bodyResponse ))
  {
    Serial.println( "########################" );
    Serial.println( "==GET OPERATION RESULTS== " );
    Serial.print( "HTTP CODE  REPLY: " );
    Serial.println( httpCodeResponse );
    Serial.println( "BODY REPLY" );
    Serial.println( bodyResponse );
    Serial.println( "########################" );
    
    // Reset the variables.
    httpCodeResponse = 0;
    // ATTENTION!! It's important to delete bodyResponse for free memory.
    delete bodyResponse;
  }
  else 
  {
    Serial.println( "@@@@@@@@@@@@@@@@@@@@@@@@@@" );
    Serial.println( "FAIL GET METHOD"); 
    Serial.println( "@@@@@@@@@@@@@@@@@@@@@@@@@@" );
  }
  
  // Make a POST petition to the server
  char data[] = "{\"title\":\"Someone here?\"}";
  if ( SIM900->Post( "private-b86c1-sim900test.apiary-mock.com", "api", "note", data, httpCodeResponse ) )
  {
    Serial.println( "########################" );
    Serial.println( "==POST OPERATION RESULTS== " );
    Serial.print( "HTTP CODE  REPLY: " );
    Serial.println( httpCodeResponse );
    Serial.println( "########################" );
  }
  
  delay(30000);
}

