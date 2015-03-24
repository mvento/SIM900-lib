/** SIM900 Basic communication library for SIM900 gsm module. 
 *
 *	This library provide easy way to make GET and POST petitions to a web server. Use to interactue Arduino with restful apis, webs, etc.
 *
 *	Released under MIT license.		
 *
 *	@author	Miquel Vento (http://www.emevento.com)
 *	@version 1.0 24/03/2015
 *
 */
#include "SIM900.h"


Connection::Connection(	const char * pinCode, 
						const char * apnName,  
						const char * apnUser, 
						const char * apnPass, 
						const uint8_t & enablePin, 
						uint8_t serialNumber, 
						uint32_t baudRate ):
	_pinCode( pinCode ),
	_apnName( apnName ),
	_apnUser( apnUser ),
	_apnPass( apnPass ),
	_enablePin( enablePin ),
	CREG_WAITING_RETRIES( 80 ),
	SAPBR_WAITING_RETRIES( 5 ),
	CGATT_WAITING_RETRIES( 30 ),
	MAX_ATRESPONSE(160)
{
	pinMode( enablePin, OUTPUT );
	
	sim900Serial = NULL;
	switch ( serialNumber )
	{	
		case 1:
			#if defined(UBRR1H)
				sim900Serial = &Serial1;
			#endif
		break;
		case 2:
			#if defined(UBRR2H)
				sim900Serial = &Serial2;
			#endif
		break;
		case 3:
			#if defined(UBRR3H)
				sim900Serial = &Serial3;
			#endif
		break;
		default:
			sim900Serial = &Serial;
	}	
	
	sim900Serial->begin( baudRate );
}


bool Connection::Configuration() 
{	
	if ( !sim900Serial )					
		return false;
		
	if ( !IsPowered() )		
		PowerOn();
	
	if ( !AT_CPIN() )
		return false;
	
	if ( !AT_CREG() )
		return false;	
			
	if ( !OpenBearer() )
		return false;
	
	return true;
}


bool Connection::Get(	const char * host, 
						const char * path, 
						const char * url, 
						uint16_t & headerHttpReply, 
						char *& bodyReply, 
						const int port )
{
	
	if ( !sim900Serial )
		return false;
		
	if ( !IsBearerOpen() )
		if ( !Configuration() )
			return false;
	
	
	if ( SendATcommand( "AT+HTTPINIT", "OK", "ERROR", 3000) != 1 )
	{
		SendATcommand( "AT+HTTPTERM", "OK", 1000 );
		if ( SendATcommand( "AT+HTTPINIT", "OK", "ERROR", 3000) != 1 )
			return false;
	}

	if ( SendATcommand("AT+HTTPPARA=\"CID\",1", "OK", 5000) != 1 )
		return false;
	
	if ( AT_HTTPPARA_URL( host, path, url ) )
	{
		if ( AT_HTTPACTION_GET( headerHttpReply ) && headerHttpReply > 0 && headerHttpReply < 600 )
		{
			delay(1000);
			if ( SendATcommand( "AT+HTTPREAD=0,100", "+HTTPREAD:", "ERROR", 30000 ) == 1 )
			{
				ReceiveData( bodyReply, 15000 );
				SendATcommand( "AT+HTTPTERM", "OK", 500 );
				return true;
			}
			
		}
	}
	
	return false;
}


bool Connection::Post(	const char * host, 
						const char * path, 
						const char * url, 
						const char * data, 
						uint16_t & headerHttpReply, 
						const int port )
{
	if ( !sim900Serial )
		return false;
		
	if ( !OpenBearer() )
		if ( !Configuration() )
			return false;

	if ( SendATcommand( "AT+HTTPINIT", "OK", "ERROR", 3000) != 1 )
	{
		SendATcommand( "AT+HTTPTERM", "OK", 1000 );
		if ( SendATcommand( "AT+HTTPINIT", "OK", "ERROR", 3000) != 1 )
			return false;
	}

	if ( SendATcommand("AT+HTTPPARA=\"CID\",1", "OK", 5000) != 1 )
		return false;

	if ( AT_HTTPDATA(strlen(data), 20000 ) )
	{
		if ( SendATcommand( data, "OK", 10000 ) )
		{
			if ( AT_HTTPPARA_URL( host, path, url ) )
			{
				if ( AT_HTTPACTION_POST( headerHttpReply ) )
				{
					SendATcommand( "AT+HTTPTERM", "OK", 500 );
					return true;
				}
			}
		}
	}

	return false;
}


void Connection::PowerOn()
{	
	digitalWrite( _enablePin, HIGH );
	delay( 1200 );
	digitalWrite( _enablePin, LOW );
	// Is necessary to wait because SIM900 sends data garbage through Serial.
	delay( 2000 );
}


void Connection::PowerOff()
{
	SendATcommand("ATE0", "", 1000 );
}


bool Connection::IsPowered()
{
	if ( SendATcommand("AT", "OK", 200 ) > 0 )
		return true;
	else
		return false;
}
	

void Connection::EchoOn( )
{
	SendATcommand("ATE1", "OK", 1000 );
}


void Connection::EchoOff( )
{
	 sim900Serial->println( "ATE0" );
}


bool Connection::AT_CPIN()
{
	int answer = SendATcommand( "AT+CPIN?", "READY", "SIM PIN", 500 );
	
	if ( answer == 1 )
	{
		return true;
	}
	else if ( answer == 2 )
	{
		CleanSerialBuffer();
		sim900Serial->print("AT+CPIN=\"");
		sim900Serial->print( _pinCode );
		sim900Serial->println( "\"" );	
	}
	
	return ReceiveATReply( "OK", 2000 );	
}


bool Connection::AT_CREG()
{
	int errorCount;
	const int totalAnswers  = 5;
	const char * expectedAnswers[totalAnswers] = { "+CREG: 0,0", "+CREG: 0,2", "+CREG: 0,4", "+CREG: 0,1","+CREG: 0,5" };
				
		
	for ( errorCount = CREG_WAITING_RETRIES; errorCount ; errorCount-- )
	{
		int answer = SendATcommand( "AT+CREG?", expectedAnswers, totalAnswers, 2000 );
		if ( answer >= 4  )
			return true;
		else if ( answer == 0 )
			return false;						
							
		delay( 1000 );
	}
			
	return false;
}


bool Connection::IsBearerOpen()
{
	if ( SendATcommand( "AT+SAPBR=2,1", "1,1", "1,3", 2000 ) == 1 )
		return true;
	
	return false;
}


bool Connection::OpenBearer()
{	
	uint8_t errorCount;
	
	if ( IsBearerOpen() )
		return true;
	
	// Waiting for GPRS Attachment
	for ( unsigned int i = CGATT_WAITING_RETRIES; i > 0; i-- )
	{
		if ( SendATcommand("AT+CGATT?", ": 0", ": 1", 1000 ) == 2 )
			break;
		delay( 1000 );
	}
	
	// Open Bearer
	if ( SendATcommand( "AT+SAPBR=1,1", "OK", "ERROR", 3000 )  == 2 )
		return false;
	
	// Wait for open the bearer										
	for ( errorCount = SAPBR_WAITING_RETRIES; errorCount > 0; errorCount-- )
	{
		if ( SendATcommand( "AT+SAPBR=2,1", "1,1", "1,3", 2000 ) == 1 )
			break;
		delay( 1000 );
	}
	
	if ( !errorCount )
		return false;
	else
		return true;
}


bool Connection::UpdateBearerInfo()
{
	// GPRS connection
	SendATcommand( "AT+SAPBR=3,1,\"Contype\",\"GPRS\"", "OK", 500 );
	
	// Set APN Name
	CleanSerialBuffer();
	sim900Serial->print( "AT+SAPBR=3,1,\"APN\",\"" );
	sim900Serial->print( _apnName );
	sim900Serial->println( "\"" );
	if ( !ReceiveATReply( "OK", 500 ) )
		return false;	
	
	// Set APN User
	CleanSerialBuffer();
	sim900Serial->print( "AT+SAPBR=3,1,\"USER\",\"" );
	sim900Serial->print( _apnUser );
	sim900Serial->println( "\"" );
	if ( !ReceiveATReply( "OK", 500 ) )
		return false;	
	
	// Set APN Passwd
	CleanSerialBuffer();
	sim900Serial->print( "AT+SAPBR=3,1,\"PWD\",\"" );
	sim900Serial->print( _apnPass );
	sim900Serial->println( "\"" );
	if ( !ReceiveATReply( "OK", 500 ) )
		return false;
		
	// Save APN Info in NVRAM
	return SendATcommand( "AT+SAPBR=5,1", "OK", 500 );
}


bool Connection::AT_HTTPACTION_GET( uint16_t & httpHeader )
{
	sim900Serial->println("AT+HTTPACTION=0");
		
	return GetHttpHeader( "+HTTPACTION:0,", httpHeader, 10000 );
}


bool Connection::AT_HTTPACTION_POST( uint16_t & httpHeader )
{
	sim900Serial->println("AT+HTTPACTION=1");
	
	return GetHttpHeader( "+HTTPACTION:1,", httpHeader, 10000 );
}


bool Connection::AT_HTTPDATA( 	const int & size, 
								const int & timeout )
{
	CleanSerialBuffer();
	sim900Serial->print( "AT+HTTPDATA=");
	sim900Serial->print( size+2 );
	sim900Serial->print( "," );
	sim900Serial->println( timeout );
	
	return ReceiveATReply( "DOWNLOAD", 10000 );
}


bool Connection::AT_HTTPPARA_URL( 	const char * host, 
									const char * path, 
									const char * url )
{
	CleanSerialBuffer();
	
	sim900Serial->print( "AT+HTTPPARA=\"URL\",\"" );
	sim900Serial->print( host );
	sim900Serial->print( "/" );
	sim900Serial->print( path );
	sim900Serial->print( "/" );
	sim900Serial->print( url );
	sim900Serial->println( "\"" );
	
	return ReceiveATReply( "OK", 10000 );	
}


bool Connection::GetHttpHeader( const char * expected_answer, 
								uint16_t & httpHeader, 
								const uint16_t & timeout )
{
	uint32_t previousTime;
	char nextChar;
	
	httpHeader = 0;

	// this loop waits for the answer
	previousTime = millis();
	
	if ( !ReceiveATReply( expected_answer, timeout ) )
		return false;
	
	for( int i = 3; i > 0; i-- )
	{	
		if ( !WaitingSerialAvailable( previousTime,timeout ) )
			return false;
			
		nextChar = sim900Serial->read();	
		
		// Convert char number to integer
		httpHeader *= 10;
		httpHeader += ( nextChar-0x30 );  // Converts ASCII character to integer
	}
	
	return true;
}


bool Connection::TimeOut( 	const uint32_t & previousTime, 
							const uint16_t & timeOut )
{
	if ( millis() - previousTime > timeOut )
		return true;
	
	return false;
}

 
uint16_t Connection::GetReceiveDataSize( const uint16_t & timeout )
{
	uint16_t dataSize = 0;
	uint32_t previousTime = millis();
	char nextChar;
	
	
	if ( !WaitingSerialAvailable( previousTime, timeout ) )
	{
		return false;
	}
	
	nextChar = sim900Serial->read();
	
	// Get Data Size
	do
	{
		// Convert char number to integer
		dataSize *= 10;
		dataSize += ( nextChar-0x30 );  // Converts ASCII character to integer
		
		if ( !WaitingSerialAvailable( previousTime, timeout ) )
			return false;
			
		nextChar = sim900Serial->read();
	} while( nextChar != 0x0D );
	

	// Consume TR character
	if ( sim900Serial->available() )
		sim900Serial->read();

	return dataSize;
}


bool Connection::WaitingSerialAvailable( 	const uint32_t & previousTime, 
											const uint16_t & timeout )
{
	while( !sim900Serial->available() )
	{
		if ( TimeOut( previousTime, timeout ) )
			return false;
	}
	return true;
}


bool Connection::ReceiveData( 	char *& bodyReply, 
								const uint16_t & timeout )
{
	char nextChar;
	uint8_t answerCount = 0;
	uint16_t dataSize = 0;
	uint32_t previousTime;
	
	previousTime = millis();
	
	dataSize = GetReceiveDataSize( 500 );
	bodyReply = new char[dataSize+1];
			
	if ( dataSize > 0)
	{
		for ( int i = dataSize; i > 0; i-- )
		{
			if ( WaitingSerialAvailable( previousTime, timeout ) )
				bodyReply[answerCount++] = sim900Serial->read();
			else
				return false;
		}
		
		bodyReply[answerCount] = '\0';
		return true;
	}
	else
	{
		return false;
	}
}


void Connection::CleanSerialBuffer()
{
	while( sim900Serial->available() > 0)
	{
		sim900Serial->read();
	}		
}


int8_t Connection::SendATcommand( 	const char* ATcommand, 
									const char* expectedAnswer, 
									const unsigned int & timeout )
{	
	const char * arrayAnswers[] = { expectedAnswer };
		
	return SendATcommand( ATcommand, arrayAnswers, 1, timeout );
}


int8_t Connection::SendATcommand(	const char* ATcommand, 
									const char* expectedAnswer1, 
									const char* expectedAnswer2, 
									const unsigned int & timeout )
{
	const char * arrayAnswers[] = { expectedAnswer1, expectedAnswer2 };
	
	return SendATcommand( ATcommand, arrayAnswers, 2, timeout );						
}


int8_t Connection::SendATcommand(	const char* ATcommand,
									const char ** expectedAnswers,
									const int & totalAnwers,
									const unsigned int & timeout )
{
	CleanSerialBuffer();
	sim900Serial->println( ATcommand );

	return ReceiveATReply( expectedAnswers, totalAnwers, timeout );
}


int8_t Connection::ReceiveATReply(	const char ** expectedAnswers,
									const int & totalAnwers,
									const unsigned int & timeout )
{
	uint8_t answerCount = 0;
	char response[MAX_ATRESPONSE];
	uint32_t previousTime;

	memset( response, '\0', MAX_ATRESPONSE );
	delay(100);

	previousTime = millis();
		
	while( ( millis() - previousTime ) < timeout )
	{
		if( !WaitingSerialAvailable( previousTime, timeout ) )
		{
			return false;
		}
		
		response[answerCount++] = sim900Serial->read();
		
		for ( int i = 0; i<totalAnwers; i++ )
		{
			if ( strstr( response, expectedAnswers[i] ) != NULL )
			{
				return i+1;
			}
		}

	}

	return 0;
}


int8_t Connection::ReceiveATReply(	const char * expectedAnswer,
									const unsigned int & timeout )
{
	const char * expectedAnswers[] = { expectedAnswer };
	
	return ReceiveATReply( expectedAnswers, 1, timeout );
}
