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
#pragma once
#ifndef __Connection_h__
#define __Connection_h__

#include <Arduino.h>

class Connection
{
public:
	/** \brief Constructuor
	 *
	 *	@param	IN	pin code of the SIM
	 *	@param	IN	apn name of your provider
	 *	@param	IN	apn user of your provider. Usually is ""
	 *	@param	IN	apn password of your privider. Usually is ""
	 *	@param	IN	enable pin for power on the board.
	 *	@param	IN	serial port of your arduino. (0 for Serial, 1 for Serial1, etc.). Defaults 0
	 *	@param	IN	baudRate for communications. Defaults 115200.
	 */
	Connection( const char * pinCode, 
				const char * apnName,  
				const char * apnUser = "", 
				const char * apnPass = "", 
				const uint8_t & enablePin = 2, 
				uint8_t serialNumber = 0, 
				uint32_t baudRate = 115200 );
	

	/** \brief Configure module to make connections.
	 *		Is necessary to executate every time after power on or reboot the module.
	 */
	bool Configuration();
	
	/** \brief Make a Get petition to the server and receive data
	 *		IMPORTANT! REMEMBER DELETE bodyReply content when you dont need more.
	 *
	 *	Request example www.example.com/api/europe/madrid/time	
	 *
	 *	@param	IN	Server domain or public IP. e.g. www.example.com OR 216.58.210.164
	 *	@param	IN	Server path eg: api
	 *	@param	IN	Server url eg: europe/madrid/time
	 *	@param	OUT	Http Reply of the request: e.g. 200, 404, etc.
	 *	@param	OUT	Reply of the request. The variable is initialized inside of method. REMEMBER DELETE when you don't need.
	 *	@param	IN	Server port. The default value is 80
	 *
	 *	@return	true if the operation ends ok
	 */
	bool Get( 	const char * host, 
				const char * path, 
				const char * url, 
				uint16_t & headerHttpReply, 
				char *& bodyReply, 
				const int port = 80 );

	/** \brief Make a Post petition to the server
	 *
	 *	Request example www.example.com/api/europe/madrid/time	
	 *
	 *	@param	IN	Server domain or public IP. e.g. www.example.com OR 216.58.210.164
	 *	@param	IN	Server path eg: api
	 *	@param	IN	Server url eg: europe/madrid/time
	 *	@param	IN	data to send.	eg: {"hour":10,"minutes":21,"seconds":13}
	 *	@param	OUT	Http Reply of the request: e.g. 200, 404, etc.
	 *	@param	IN	Server port. The default value is 80
	 *
	 *	@return	true if the operation ends ok
	 */
	bool Post( 	const char * host, 
				const char * path, 
				const char * url, 
				const char * data, 
				uint16_t & headerHttpReply, 
				const int port = 80 );
	
	/** \brief Turn on the GPRS module.
	 *	
	 */
	void PowerOn();
	
	/** \brief Turns off the GPRS module
	 *
	 */
	void PowerOff();
	
protected:	
	/** \brief Check if the GPRS module is on.
	 *
	 *	@return true if the module is on
	 */
	bool IsPowered();
		
	/** \brief Enables SIM900 AT command echo
	 *	
	 */
	void EchoOn( );
	/** \brief Disables SIM900 AT Command echo
	 *	
	 */
	void EchoOff( );
	
	
	/** \brief Introduces the pin code if is necessary
	 *	
	 */
	bool AT_CPIN();
	

	/** \brief Check the status of the ME registration
	 *
	 *	-Correct Answers
	 *		+CREG: 0,1 -> Registered, home network
	 *		+CREG: 0,5 -> Registered, roaming
	 *	-Waiting Answers
	 *		+CREG: 0,0 -> Not registered, MT is not currently searching
	 *		+CREG: 0,2 -> Not registered, MT is currently searching
	 *		+CREG: 0,4 -> Unknow
	 *
	 *	@return true if the action ends OK
	 */
	bool AT_CREG();

	/** \brief Check the state of the bearer
	 *
	 *	@return true if is opened
	 */
	bool IsBearerOpen();
	

	/** \brief Bearer settings for applications based on ip
	 *
	 *	@return true if the action ends OK
	 */
	bool OpenBearer();
	
	/** \brief Update the information of the APN provider. This action is only necessary if you changes the SIM service provider
	 *
	 *	@return true if the info is introduced and storaged correctly
	 */
	bool UpdateBearerInfo();

	/** \brief Makes a Get petition  
	 *
	 */
	bool AT_HTTPACTION_GET( uint16_t & httpHeader );

	/** \brief Makes a Post petition
	 *
	 */
	bool AT_HTTPACTION_POST( uint16_t & httpHeader );
	
	/** \brief Sends the content for POST petition
	 *
	 *	@param	IN	size of the data
	 *	@param	IN	timeout for the operation
	 *
	 *	@return	true if the operation ends correctly
	 */
	bool AT_HTTPDATA( 	const int & size, 
						const int & timeout );
	
	/** \brief Set the http parameters
	 *
	 *	@param	IN	host of the server
	 *	@param	IN	path of the url
	 *	@param	IN	the other part of the url
	 *
	 *	@return	true if the operation ends correctly
	 */
	bool AT_HTTPPARA_URL( 	const char * host, 
							const char * path, 
							const char * url );
	
	/** \brief Receive the httpheader of a GET or POST petition
	 *
	 *	Reply example: +HTTPACTION:1,201,0
	 *
	 *	@param	IN	expected answere (HTTPACTION:*,)
	 *	@param	OUT	httpHeader received for operation
	 *	@param	IN	timeout for the operation
	 *
	 *	@return	true if the operation ends correctly
	 */
	bool GetHttpHeader( const char * expected_answer, 
						uint16_t & httpHeader, 
						const uint16_t & timeout );
	
	/** \brief	Calculates if time for operation is run out
	 *
	 *	@return	true after timing out
	 */
	bool TimeOut( 	const uint32_t & previousTime, 
					const uint16_t & timeOut );
	
	
	/** \brief Get the size of the received data from server.
	 *			Execute this method INMEDIATLY after AT+HTTPREAD operation
	 */ 
	uint16_t GetReceiveDataSize( const uint16_t & timeout );

	/** \brief Waits for serial data, in accotated time.
	 *
	 */
	bool WaitingSerialAvailable( 	const uint32_t & previousTime, 
									const uint16_t & timeout );	
	
	/** \brief Receive data from server after Get request.
	 *			Get the data after send AT+HTTPREAD command
	 *
	 *	@param	OUT	bodyReply	string of server answer
	 *
	 *	@return	true if operation finish ok
	 */
	bool ReceiveData(	char *& bodyReply, 
						const uint16_t & timeout );
	
	/** \brief Cleans Serial input buffer.
	 *
	 */
	void CleanSerialBuffer();

	/** \brief Send AT commmand and check the answer
	 *
	 *	@return 0 if the recuest isn't correct
	 */
	int8_t SendATcommand( 	const char* ATcommand, 
							const char* expectedAnswer, 
							const unsigned int & timeout );

	/** \brief Send AT command and check the two possible answers
	 *
	 *	@return the number of the answer or 0 if it don't exist
	 */
	int8_t SendATcommand(	const char* ATcommand, 
							const char* expectedAnswer1, 
							const char* expectedAnswer2, 
							const unsigned int & timeout );

	/** \brief Send AT command and check the N possible answers
	 *
	 *	@param	IN	ATcommand		AT command to send
	 *	@param	IN	expectedAnswers	String array for posibles answers
	 *	@param	IN	totalAnswers	size of expectedAnswers array
	 *	@param	IN	timeout			max size for finish the operation
	 *
	 *	@return the number of the answer or 0 if it don't exist
	 */
	int8_t SendATcommand(	const char* ATcommand,
							const char ** expectedAnswers,
							const int & totalAnwers,
							const unsigned int & timeout );	
	
	/** \brief Receive the response after AT Command Send.
	 *
	 *	@param	IN	expectedAnswers	String array for posibles answers
	 *	@param	IN	totalAnswers	size of expectedAnswers array
	 *	@param	IN	timeout			max size for finish the operation
	 */
	int8_t ReceiveATReply(	const char ** expectedAnswers,
							const int & totalAnwers,
							const unsigned int & timeout );	
	
	/** \brief Receive the response after AT Command Send.
	 *
	 *	@param	IN	expectedAnswer	expected answer to receive
	 *	@param	IN	timeout			max size for finish the operation
	 */
	int8_t ReceiveATReply(	const char * expectedAnswer,
							const unsigned int & timeout );

private:
	const char * _pinCode;
	const char * _apnName;
	const char * _apnUser;
	const char * _apnPass;
	
	const uint8_t _enablePin;
	
	// Configuration
	const int CREG_WAITING_RETRIES;
	const int SAPBR_WAITING_RETRIES;
	const int CGATT_WAITING_RETRIES;
	const int MAX_ATRESPONSE;
	
	HardwareSerial * sim900Serial;
};

#endif
