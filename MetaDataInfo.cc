/**
@file MetaDataInfo.cc
@author Eugene Nelson
@breif The implamentation file for the MetaDataInfo class.
@version    1.0 Eugene Nelson
            Originally developed ( 9 - 19 - 17 )
*/

#include "MetaDataInfo.hh"
#include "MemoryFunction.hh"


bool operator > ( const process &P1, const process &P2 )
{
    return P1.priority > P2.priority;
}

bool operator < ( const process &P1, const process &P2 )
{
    return P1.priority < P2.priority;
}
// Thread Functions
/////////////////////////////////////////////////////////////////////////////
void* threadInput( void* threadarg )
{
	struct thread_data* data = (struct thread_data*) threadarg;
	data->executionTime = timer( data->tempProcessRunTime, data->initTime );
	pthread_exit( NULL );
}

void* threadOutput( void* threadarg )
{
	struct thread_data* data = (struct thread_data*) threadarg;
	data->executionTime = timer( data->tempProcessRunTime, data->initTime );
	pthread_exit( NULL );
}

double timer( long timeToWait, timeval& initTime )
{
	double time = 0;

	timeval start, end;
	gettimeofday( &start, NULL );

	while ( getWaitTime( start ) < timeToWait )
	{
		;
	}


	gettimeofday( &end, NULL );

	int sec, usec;

	sec =  end.tv_sec - initTime.tv_sec;
	usec = end.tv_usec - initTime.tv_usec; 

	if( usec < 0 )
	{
		usec += 1000000;
		sec -= 1;
	}

	time += (double) ( sec + (double)usec/1000000 );

	return time;
}

long getWaitTime( timeval& start )
{
	timeval current;
	gettimeofday( &current, NULL );

	int sec, usec;

	sec =  current.tv_sec - start.tv_sec;
	usec = current.tv_usec - start.tv_usec; 

	if( usec < 0 )
	{
		usec += 1000000;
		sec -= 1;
	}

	return (long)( sec*1000000 + (double)usec ); 
}

void wait( semaphore* S )
{
	S->m.lock( );
	while( S->value < 0 )
	{
		;
	}
}

void signal( semaphore *S )
{
	S->m.lock( );
	S->value++;
	if( S->value > S->size )
		S->value = 1;
	S->m.release( );
}
/////////////////////////////////////////////////////////////////////////////
// Constructors/Deconstructors
/////////////////////////////////////////////////////////////////////////////
MetaDataInfo::MetaDataInfo( char* fileName, char* schedulingCode )
{
	cout << fixed << showpoint;
	cout << setprecision( 6 ); 
    if( strstr( fileName, ".mdf" ) == NULL )
    {
        cout << "Error reading meta data file: Incorrect extention." << endl;
        return;
    }

    ifstream fin;
    fin.open( fileName );

    if( !fin.good( ) )
    {
        cout << "Error reading meta data file: File not good." << endl;
        fin.close( );
        return;
    }

    strcpy( aProcessSchedulingCode, schedulingCode );
    processData tempData;

    bool fileStatus = 1;
    char tempLine[ 300 ] = {'\0'};
    fin.getline( tempLine, 300, '\n' );
    do {
        if( !ParseLine( tempLine, tempData ) )
        {
            fileStatus = 0;
        }
        fin.getline( tempLine, 300, '\n' );
    } while( fileStatus );

    fin.close();
} // end Constructor

MetaDataInfo::~MetaDataInfo( )
{
    ;
} // end Deconstructor

/**<
Processes the data from the Meta Data file and logs it to appropriate output

@pre ConfigFile has all necessary information to process.
@post Simulation will have been run and output logged to appropriate outlets.
@param configFile   The configuration file with necessary info to preform
                    the simulation.
@return void
****************************************************************************/
void MetaDataInfo::ProcessData( ConfigFileInput& configFile,
								PCB& state )
{
    // Variable declarations/initialization
    /////////////////////////////////////////////////////////////////////////
    char tempLogFilePath[ 30 ];
    char tempLogDirectory[ 30 ];
    char* tempLogFilePathPtr;
    char logSpecification = configFile.GetLogOutputSpecification( );

    strcpy( tempLogFilePath, configFile.GetLogFilePath( ) );
    if( logSpecification == 'F' || logSpecification == 'B' ) 
    {
        if( strpbrk( tempLogFilePath, "/" ) != NULL )
        {
            tempLogFilePathPtr = strtok( tempLogFilePath, "/" );
            strcpy( tempLogDirectory, "mkdir -p " );
            strcat( tempLogDirectory, tempLogFilePathPtr );

            const int dir_err = system( tempLogDirectory );
            if( -1 == dir_err )
            {
                cout << "Error creating directory." << endl;
                exit(1);
            }
        }   
    }

    ofstream logFile( configFile.GetLogFilePath( ) );
    logFile << fixed << showpoint;
    logFile << setprecision( 6 );
    if( !logFile.good( ) )
    {
        cout << "Error creating LogFile. Please try again." << endl;
        logFile.close();
        return;
    } 

    char tempMessage[ 500 ];
    
    /*
    char tempProcessName[ 30 ];
    char tempProcessValue[ 5 ];
	*/
	timeval initTime;
    gettimeofday( &initTime, NULL );

    queue<MetaDataInfoNode> tempStorageQueue;
    /*
    // Process the Configuration Data to output to Log
    /////////////////////////////////////////////////////////////////////////
    for( int i = 1; i < configFile.GetNumberOfProcesses( ) + 1; i++ )
    {
        tempMessage[ 0 ] = '\0';
        strcpy( tempProcessName, configFile.GetProcessName( i ) );
        itoa(   configFile.GetProcessValue( tempProcessName ), 
                tempProcessValue, 10 );

        for( int j = 0; j < (int) strlen( tempProcessName ); j++ )
        {
            if( j == 0 || tempProcessName[ j - 1 ] == ' ' )
            {
                tempProcessName[ j ] = toupper( tempProcessName[ j ] );
            }
        }

        strcat( tempMessage, tempProcessName );
        strcat( tempMessage, " = " );
        strcat( tempMessage, tempProcessValue );

        if( strncmp( tempProcessName, "System", 6 ) == 0 )
        {
            strcat( tempMessage, " kbytes\n" );
        }
        else
        {
            strcat( tempMessage, " ms/cycle\n" );
        }

        LogOutput( logSpecification, tempMessage, logFile ); 
    }

    strcpy( tempMessage, "Logged to: " );
    if( logSpecification == 'M' || logSpecification == 'B' )
    {
        strcat( tempMessage, "monitor" );
    }
    if( logSpecification == 'B' )
    {
        strcat( tempMessage, " and " );
    }
    if( logSpecification == 'F' || logSpecification == 'B' )
    {
        strcat( tempMessage, configFile.GetLogFilePath( ) );
    }

    strcat( tempMessage, "\n\nMeta-Data Metrics\n" );
    LogOutput( logSpecification, tempMessage, logFile ); 
    */
    // More Variable Initializations for processing the Meta Data
    /////////////////////////////////////////////////////////////////////////

    char tempToken[ 20 ];
    //char tempToken2[ 10 ];

    char tempMetaDataCode = '\0';
    char tempMetaDataDescriptor[ 30 ];
    int tempNumberOfCycles = 0;
    int tempProcessRunTime = 0;
    int tempErrorCode = 0;
    int threadNum = 0;
    unsigned int startMemory = 0;
	unsigned int totalMemory = configFile.GetSystemMemory( );
	unsigned int memoryBlockSize = configFile.GetMemoryBlockSize( );

	semaphore S[ 15 ];

	for( int i = 0; i < configFile.GetNumberOfProcesses( ); i++ )
	{
		S[ i ].size = configFile.GetProcessQuantity( configFile.GetProcessName( i ) );
		S[ i ].value = 0; 
	}

	pthread_t threads[ 40 ];
	struct thread_data data[ 40 ];

    // Begin processing Meta Data
    /////////////////////////////////////////////////////////////////////////

    while( !aPriorityQueueOfProcesses.empty( ) )
    {
        tempMetaDataCode = '\0';
        tempMetaDataDescriptor[ 0 ] = '\0';
        tempNumberOfCycles = 0;
        tempProcessRunTime = 0;
        tempErrorCode = 0;

        // Load Meta Data info into buffers
        /////////////////////////////////////////////////////////////////////
        tempMetaDataCode = aPriorityQueueOfProcesses.top( ).metaDataQueue.front( ).aMetaDataCode;
        strcpy(	tempMetaDataDescriptor, 
        		aPriorityQueueOfProcesses.top( ).metaDataQueue.front( ).aMetaDataDescriptor );
        tempNumberOfCycles = aPriorityQueueOfProcesses.top( ).metaDataQueue.front( ).aNumberOfCycles;
        tempErrorCode = aPriorityQueueOfProcesses.top( ).metaDataQueue.front( ).aErrorCode;

        // Pop the meta data from the top process of the process queue
        /////////////////////////////////////////////////////////////////////
		process tempProcess = aPriorityQueueOfProcesses.top( );
        aPriorityQueueOfProcesses.pop( );

        tempProcess.metaDataQueue.pop( );
        aPriorityQueueOfProcesses.push( tempProcess );

        if( tempErrorCode != 0 )
        	;
        else
        {
	        // Check that code match with appropriate descriptor and get runtime
	        /////////////////////////////////////////////////////////////////////
	        switch( tempMetaDataCode )
	        {
	            case 'S':
	                if( strcmp( tempMetaDataDescriptor, "start" ) == 0 )
	                {
            	        if( state.processState == 1 )
					    {
					    	strcpy( tempMessage, " - Simulator program starting\n" );
					    	LogTime( logSpecification, timer( 0, initTime ), logFile );
					    	LogOutput( logSpecification, tempMessage, logFile);
					    	state.processState++;
					    }
	                }
	                else if( strcmp( tempMetaDataDescriptor, "end" ) == 0 )
	                {
                    	if( state.processState == 2 )
					    {
					    	strcpy( tempMessage, " - Simulator program ending\n" );
					    	LogTime( logSpecification, timer( 0, initTime ), logFile );
					    	LogOutput( logSpecification, tempMessage, logFile);

					    	state.processState = 5;

					    	if( aPriorityQueueOfProcesses.top( ).metaDataQueue.empty( ) )
					    	{
					    		aPriorityQueueOfProcesses.pop( );
					    	}
					    }
	                }
	                else
	                    tempErrorCode = 41;

	                break;
	            case 'A':
	                if( strcmp( tempMetaDataDescriptor, "start" ) == 0 )
	                {
	                    if( tempNumberOfCycles == 0 )
	                    {
	                    	// Move PCB to READY mode
						    if( state.processState == 2 )
						    {
						    	strcpy( tempMessage, " - OS: preparing process " );
						    	strcat( tempMessage, itoa( aPriorityQueueOfProcesses.top().processNum, tempToken, 10 ) );
						    	strcat( tempMessage, "\n" );
		
						    	LogTime( logSpecification, (double) timer( 0, initTime ) , logFile );
						    	LogOutput( logSpecification, tempMessage, logFile);
						    	state.processState++;
						    }
							// Move PCB to RUNNING mode
						    if( state.processState == 3 )
						    {
						    	strcpy( tempMessage, " - OS: starting process " );
						    	strcat( tempMessage, itoa( aPriorityQueueOfProcesses.top().processNum, tempToken, 10 ) );
						    	strcat( tempMessage, "\n" );

						    	LogTime( logSpecification, timer( 0, initTime ), logFile );
						    	LogOutput( logSpecification, tempMessage, logFile);
						    }
	                    }
	                    else
	                        tempErrorCode = 51;
	                }
	                else if( strcmp( tempMetaDataDescriptor, "end" ) == 0 )
	                {
	                    if( tempNumberOfCycles == 0 )
	                    {
	              			// Move PCB to READY mode
						    if( state.processState == 3 )
						    {
						    	strcpy( tempMessage, " - End process " );
						    	strcat( tempMessage, itoa( aPriorityQueueOfProcesses.top().processNum, tempToken, 10 ) );
						    	strcat( tempMessage, "\n" );

						    	LogTime( logSpecification, timer( 0, initTime ), logFile );
						    	LogOutput( logSpecification, tempMessage, logFile);
						    	state.processState--;
						    	if( aPriorityQueueOfProcesses.top( ).metaDataQueue.empty( ) )
						    	{
						    		aPriorityQueueOfProcesses.pop( );
						    	}
						    }	
	                    }
	                    else
	                        tempErrorCode = 51;
	                }
	                else
	                    tempErrorCode = 41;

	                break;
	            case 'P':
	                if( strcmp( tempMetaDataDescriptor, "run" ) == 0 )
	                {
	                    tempProcessRunTime = tempNumberOfCycles * 
	                                    configFile.GetProcessValue( "processor" ) * 1000;
					    if( state.processState == 3 )
					    {
					    	strcpy( tempMessage, " - Process " );
						    strcat( tempMessage, itoa( aPriorityQueueOfProcesses.top().processNum, tempToken, 10 ) );
						    strcat( tempMessage, ": start processing action\n" );

					    	LogTime( logSpecification, timer( 0, initTime ), logFile );
					    	LogOutput( logSpecification, tempMessage, logFile);
					    	
					    	strcpy( tempMessage, " - Process " );
						    strcat( tempMessage, itoa( aPriorityQueueOfProcesses.top().processNum, tempToken, 10 ) );
						    strcat( tempMessage, ": end processing action\n" );

					    	LogTime( logSpecification, timer( tempProcessRunTime, initTime ), logFile );
					    	LogOutput( logSpecification, tempMessage, logFile);
					    	
					    }	
	                }
	                else
	                    tempErrorCode = 51;

	                break;
	            case 'I':
	                if( tempNumberOfCycles != 0 )
	                {
	                    if( configFile.GetProcessValue( tempMetaDataDescriptor ) 
	                        == -1 )
	                        tempErrorCode = 41;
	                    else
	                        tempProcessRunTime = tempNumberOfCycles * 
	                                    configFile.GetProcessValue( tempMetaDataDescriptor ) * 1000;
	                    if( state.processState == 3 )
	                    {
	                    	data[ threadNum ].initTime = initTime;
	                    	data[ threadNum ].tempProcessRunTime = tempProcessRunTime;
							
							strcpy( tempMessage, " - Process " );
						    strcat( tempMessage, itoa( aPriorityQueueOfProcesses.top().processNum, tempToken, 10 ) );
						    strcat( tempMessage, ": start " );
							strcat( tempMessage, tempMetaDataDescriptor );
							strcat( tempMessage, " input\n" );
							LogTime( logSpecification, timer( 0, initTime ), logFile );
							LogOutput( logSpecification, tempMessage, logFile);
	                    	
							wait( &S[ configFile.GetProcessNumber( tempMetaDataDescriptor ) ] );
							state.processState = 4;
	                    	pthread_create( &threads[ threadNum ], NULL, threadInput, ( void* ) &data[ threadNum ] );
	                    	pthread_join( threads[ threadNum ], NULL );
	                    	signal( &S[ configFile.GetProcessNumber( tempMetaDataDescriptor ) ] );
	                    	state.processState = 3;

							strcpy( tempMessage, " - Process " );
						    strcat( tempMessage, itoa( aPriorityQueueOfProcesses.top().processNum, tempToken, 10 ) );
						    strcat( tempMessage, ": end " );
							strcat( tempMessage, tempMetaDataDescriptor );
							strcat( tempMessage, " input" );

							if( S[ configFile.GetProcessNumber( tempMetaDataDescriptor ) ].size > 1 )
							{
								strcat( tempMessage, " on " );
								if( strcmp( tempMetaDataDescriptor, "hard drive" ) == 0 )
									strcat( tempMessage, "HDD " );
								else if( strcmp( tempMetaDataDescriptor, "printer" ) == 0 )
									strcat( tempMessage, "PRNTR " );
								else if( strcmp( tempMetaDataDescriptor, "keyboard" ) == 0 )
									strcat( tempMessage, "KBD " );
								else
									strcat( tempMessage, "device " );

								strcat( tempMessage, 
									itoa( S[ configFile.GetProcessNumber( tempMetaDataDescriptor ) ].value - 1, 
											tempToken, 10 ) );

							}

							strcat( tempMessage, "\n" );

	                    	LogTime( logSpecification, data[ threadNum ].executionTime, logFile );
							LogOutput( logSpecification, tempMessage, logFile);

	                    	threadNum++;
	                    }
	                }
	                else
	                    tempErrorCode = 41;

	                break;
	            case 'O':
	                if( tempNumberOfCycles != 0 )
	                {
	                    if( configFile.GetProcessValue( tempMetaDataDescriptor ) 
	                        == -1 )
	                        tempErrorCode = 41;
	                    else
	                        tempProcessRunTime = tempNumberOfCycles * 
	                                    configFile.GetProcessValue( tempMetaDataDescriptor ) * 1000;

	                    if( state.processState == 3 )
	                    {
	                    	data[ threadNum ].initTime = initTime;
	                    	data[ threadNum ].tempProcessRunTime = tempProcessRunTime;


							
							strcpy( tempMessage, " - Process " );
						    strcat( tempMessage, itoa( aPriorityQueueOfProcesses.top().processNum, tempToken, 10 ) );
						    strcat( tempMessage, ": start " );
							strcat( tempMessage, tempMetaDataDescriptor );
							strcat( tempMessage, " output\n" );
							LogTime( logSpecification, timer( 0, initTime ), logFile );
							LogOutput( logSpecification, tempMessage, logFile);
	                    	
	                    	wait( &S[ configFile.GetProcessNumber( tempMetaDataDescriptor ) ] );
							state.processState = 4;
	                    	pthread_create( &threads[ threadNum ], NULL, threadInput, ( void* ) &data[ threadNum ] );
	                    	pthread_join( threads[ threadNum ], NULL );
	                    	signal( &S[ configFile.GetProcessNumber( tempMetaDataDescriptor ) ] );
	                    	state.processState = 3;
	                    	
							strcpy( tempMessage, " - Process " );
						    strcat( tempMessage, itoa( aPriorityQueueOfProcesses.top().processNum, tempToken, 10 ) );
						    strcat( tempMessage, ": end " );
							strcat( tempMessage, tempMetaDataDescriptor );
							strcat( tempMessage, " output" );

							if( S[ configFile.GetProcessNumber( tempMetaDataDescriptor ) ].size > 1 )
							{
								strcat( tempMessage, " on " );
								if( strcmp( tempMetaDataDescriptor, "hard drive" ) == 0 )
									strcat( tempMessage, "HDD " );
								else if( strcmp( tempMetaDataDescriptor, "printer" ) == 0 )
									strcat( tempMessage, "PRNTR " );
								else if( strcmp( tempMetaDataDescriptor, "keyboard" ) == 0 )
									strcat( tempMessage, "KBD " );
								else
									strcat( tempMessage, "device " );

								strcat( tempMessage, 
									itoa( S[ configFile.GetProcessNumber( tempMetaDataDescriptor ) ].value - 1, 
											tempToken, 10 ) );
							}

							strcat( tempMessage, "\n" );
	                    	LogTime( logSpecification, data[ threadNum ].executionTime, logFile );
							LogOutput( logSpecification, tempMessage, logFile);

	                    	threadNum++;
	                    }
	                }
	                else
	                    tempErrorCode = 41;

	                break;
	            case 'M':
	                if( tempNumberOfCycles > 0 )
	                        tempProcessRunTime = tempNumberOfCycles *
	                                    configFile.GetProcessValue( "memory" ) * 1000;
	                else
	                {
	                    tempErrorCode = 51;
	                	break;
	                }

	                if( strcmp( tempMetaDataDescriptor, "block" ) == 0 )
	                {
	                    if( state.processState == 3 )
					    {
					    	strcpy( tempMessage, " - Process " );
						    strcat( tempMessage, itoa( aPriorityQueueOfProcesses.top().processNum, tempToken, 10 ) );
						    strcat( tempMessage, ": start memory blocking\n" );
					    	LogTime( logSpecification, timer( 0, initTime ), logFile );
					    	LogOutput( logSpecification, tempMessage, logFile);
					    	
					    	strcpy( tempMessage, " - Process " );
						    strcat( tempMessage, itoa( aPriorityQueueOfProcesses.top().processNum, tempToken, 10 ) );
						    strcat( tempMessage, ": end memory blocking\n" );
					    	LogTime( logSpecification, timer( tempProcessRunTime, initTime ), logFile );
					    	LogOutput( logSpecification, tempMessage, logFile);
					    	
					    }
	                }
	                else if( strcmp( tempMetaDataDescriptor, "allocate" ) == 0 )
	                {
	               		if( state.processState == 3 )
					    {
					    	strcpy( tempMessage, " - Process " );
						    strcat( tempMessage, itoa( aPriorityQueueOfProcesses.top().processNum, tempToken, 10 ) );
						    strcat( tempMessage, ": start allocating memory\n" );
					    	LogTime( logSpecification, timer( 0, initTime ), logFile );
					    	LogOutput( logSpecification, tempMessage, logFile);
					    	
					    	strcpy( tempMessage, " - Process " );
						    strcat( tempMessage, itoa( aPriorityQueueOfProcesses.top().processNum, tempToken, 10 ) );
						    strcat( tempMessage, ": memory allocated at " );
					    	strcat( tempMessage, itoa( (int) allocateMemory( totalMemory, startMemory, memoryBlockSize ), tempToken, 16 ) );
					    	strcat( tempMessage, "\n" );
					    	LogTime( logSpecification, timer( tempProcessRunTime, initTime ), logFile );
					    	LogOutput( logSpecification, tempMessage, logFile);
					    	
					    }
	                }
	                else
	                    tempErrorCode = 41;

	                break;
	            default:
	                tempErrorCode = 31;

	        }
	    }

        // Prepare the line for the Log output
        /////////////////////////////////////////////////////////////////////
        /*if( tempErrorCode != 0 )
        {
            
            //ProcessErrorCode( logSpecification, tempErrorCode, logFile );
        }*/
    }

    logFile.close( );

/*    // Return queue to original state
    /////////////////////////////////////////////////////////////////////////
    while( !tempStorageQueue.empty( ) )
    {
        aQueueOfMetaData.push( ( tempStorageQueue.front( ) ) );
        tempStorageQueue.pop( ); 
    }*/
} // End ParseData

/**<
Parses the line from Meta Data and stores the tokens in nodes in the queue

@pre None.
@post   Meta Data line will be parsed and all important information will be
        stored in nodes within the queue of processes to be simulated later.
@param lineToParse  The line from the MetaData file that will be parsed
@return The status of the parse, indicating errors in the parsing process.
@note   Will also set error codes for any potentially troublesome processes
        so that they may be delt with later, without crashing the program.
****************************************************************************/
bool MetaDataInfo::ParseLine( char lineToParse[ ], processData& pD )
{
    // Variable declarations/initialization
    /////////////////////////////////////////////////////////////////////////
    char tempHelper[ 300 ] = {'\0'};
    
    char* tempHelperPtr = new char[ 300 ];
    char* tempHelperPtrAnchor = tempHelperPtr;
    
    char tempMetaDataCode = '\0';
    char tempMetaDataDescriptor[ 30 ] = {'\0'};
    
    int tempNumberOfCycles = -1;
    int tempErrorCode = 0;
    
    MetaDataInfoNode tempNode;

    RemoveSpaces( lineToParse );
    strcpy( tempHelper, lineToParse );
    strcpy( tempHelperPtr, tempHelper );

    // Begin parsing the line
    /////////////////////////////////////////////////////////////////////////
    int iterator = 0;
    while( tempHelperPtr != NULL )
    {
        tempMetaDataCode = '\0';
        tempMetaDataDescriptor[ 0 ] = '\0';
        tempNumberOfCycles = -1;
        tempErrorCode = 0;

        if( strncmp( lineToParse, "StartProgramMeta-Data", 21 ) == 0 ) 
        {
        	tempHelperPtr = NULL;
        }
        else if( strncmp( lineToParse, "EndProgramMeta-Data", 19 ) == 0 )
        {
     		// Add a S(start) task to the front of the first process
        	if( aPriorityQueueOfProcesses.empty( ) )
        	{
        		process* tempProcess = new process;
        		aPriorityQueueOfProcesses.push( *tempProcess );
        	}

        	priority_queue<process, vector<process>, greater<process> > tempPQueue;
        	process tempProcess = aPriorityQueueOfProcesses.top( );
        	aPriorityQueueOfProcesses.pop( );

        	queue<MetaDataInfoNode> tempMetaDataQueue;

			tempNode.aMetaDataCode = 'S';
			strcpy( tempNode.aMetaDataDescriptor, "start" );
	        tempNode.aNumberOfCycles = 0;
	        tempNode.aErrorCode = 0;

	        tempMetaDataQueue.push( tempNode );

        	while( !tempProcess.metaDataQueue.empty( ) )
        	{
        		tempMetaDataQueue.push( tempProcess.metaDataQueue.front( ) );
        		tempProcess.metaDataQueue.pop( );
        	}
        	while( !tempMetaDataQueue.empty( ) )
        	{
        		tempProcess.metaDataQueue.push( tempMetaDataQueue.front( ) );
        		tempMetaDataQueue.pop( );
        	}

        	if( !aPriorityQueueOfProcesses.empty( ) )
        	{
        		tempPQueue.push( tempProcess );
        	}

        	while( !aPriorityQueueOfProcesses.empty( ) )
        	{
        		tempProcess = aPriorityQueueOfProcesses.top( );
        		aPriorityQueueOfProcesses.pop( );
        		if( !aPriorityQueueOfProcesses.empty( ) )
        		{
        			tempPQueue.push( tempProcess );
        		}
        	}

        	// Add a S(end) task to the back of the last process
        	strcpy( tempNode.aMetaDataDescriptor, "end" );
        	tempProcess.metaDataQueue.push( tempNode );

        	tempPQueue.push( tempProcess );

        	// Put the temp pQueue back into the member pQueue
        	while( !tempPQueue.empty( ) )
        	{
        		aPriorityQueueOfProcesses.push( tempPQueue.top( ) );
        		tempPQueue.pop( );
        	}

        	return false;
        }
        else
        {
	        tempHelperPtr = strtok( tempHelperPtr, ".,:;\0" );
	        tempHelperPtr = strtok( tempHelperPtr, "(" );

	        tempMetaDataCode = ( tempHelperPtr[ 0 ] );
	        if( tempMetaDataCode == '\0' )
	        {
	            tempErrorCode = 33;
	        }
	        else if( tempMetaDataCode > 'Z' || tempMetaDataCode < 'A' )
	        {
	            tempErrorCode = 32;
	        }

	        tempHelperPtr = strtok( NULL, ")" );
	        if( tempHelperPtr == NULL )
	        {
	            tempErrorCode = 42;
	        }
	        else
	        {
	            strcpy( tempMetaDataDescriptor, tempHelperPtr );
	        }

	        tempHelperPtr = strtok( NULL, ":" );
	        if( tempHelperPtr == NULL )
	        {
	            tempErrorCode = 53;
	        }
	        else if( tempHelperPtr[ 0 ] > '9' || tempHelperPtr[ 0 ] < '0' )
	        {
	            tempErrorCode = 54;
	        }
	        else
	        {
	            tempNumberOfCycles = atoi( tempHelperPtr );
	        }

	        if( tempNumberOfCycles == -1 )
	        {
	        	tempErrorCode = 52;
	        }
	        else if( tempNumberOfCycles < 0 )
	        {
	            tempErrorCode = 53;
	        }

	        else if( tempMetaDataCode == 'A' || tempMetaDataCode == 'S' )
	        {
	        	if( strcmp( tempMetaDataDescriptor, "start" ) == 0 )
	        	{
	        		if( tempMetaDataCode == 'S' )
			        {
			        	;
			        }
			        // Make a new process as long as one does not already exist
	        		else if( pD.tempProcess == NULL )
	        		{
	        			pD.tempProcess = new process;
		        		pD.processNum++;
		        		pD.tempProcess->processNum = pD.processNum;

		        		tempNode.aMetaDataCode = tempMetaDataCode;
				        strcpy( tempNode.aMetaDataDescriptor, tempMetaDataDescriptor );
				        tempNode.aNumberOfCycles = tempNumberOfCycles;
				        tempNode.aErrorCode = tempErrorCode;

		        		pD.tempProcess->metaDataQueue.push( tempNode );
		        	}
	        	}
	        	else if( strcmp( tempMetaDataDescriptor, "end" ) == 0 )
	        	{
	        		if( pD.tempProcess == NULL )
	        		{
	        			;
	        		}
	        		else
	        		{
		        		if( strcmp( aProcessSchedulingCode, "FIFO" ) == 0 )
		        		{
		        			pD.tempProcess->priority = pD.processNum;
		        		}
		        		else if( strcmp( aProcessSchedulingCode, "SJF" ) == 0 )
		        		{
		        			pD.tempProcess->priority = pD.taskNum;
		        		}
		        		else if( strcmp( aProcessSchedulingCode, "PS" ) == 0 )
		        		{
		        			pD.tempProcess->priority = ( -1 * pD.inputOutputNum );
		        		}

						tempNode.aMetaDataCode = 'A';
				        strcpy( tempNode.aMetaDataDescriptor, "end" );
				        tempNode.aNumberOfCycles = 0;
				        tempNode.aErrorCode = 0;

				        pD.tempProcess->metaDataQueue.push( tempNode );

		        		aPriorityQueueOfProcesses.push( *( pD.tempProcess ) );
		        		delete pD.tempProcess;
		        		pD.tempProcess = NULL;
		        		pD.taskNum = 0;
		        		pD.inputOutputNum = 0;
		        	}
	        	}
	        }
	        else
	        {
		        tempNode.aMetaDataCode = tempMetaDataCode;
		        strcpy( tempNode.aMetaDataDescriptor, tempMetaDataDescriptor );
		        tempNode.aNumberOfCycles = tempNumberOfCycles;
		        tempNode.aErrorCode = tempErrorCode;

		        if( pD.tempProcess != NULL )
		        {
		    		if( tempMetaDataCode == 'I' || tempMetaDataCode == 'O' )
			    	{
			    		pD.inputOutputNum++;
			    	}

			    	pD.taskNum++;

		        	pD.tempProcess->metaDataQueue.push( tempNode );	
			    }
		    }
	        tempHelperPtr = tempHelperPtrAnchor;
	        strcpy( tempHelperPtr, tempHelper );
	        tempHelperPtr = strtok( tempHelperPtr, ".,:;\0" );

	        for( int i = 0; i < iterator; i++ )
	        {
	            tempHelperPtr = strtok( NULL, ",.:;\0" );
	        }

	        tempHelperPtr = strtok( NULL, "\0" );

	        iterator++;
	    }
    }
    delete tempHelperPtrAnchor;

    tempHelperPtr = NULL;
    tempHelperPtrAnchor = NULL;

    return true;
} // end ParseLine

/**<
Helper function to make parsing easier

@pre    None.
@post   The specified string will have all space characters removed.
@param  lineToRemoveSpaces The string who's spaces will be removed.
@return void
****************************************************************************/
void MetaDataInfo::RemoveSpaces( char lineToRemoveSpaces[ ] )
{
    for( int i = 0; i < ( int ) strlen( lineToRemoveSpaces ); i++ )
    {
        if( lineToRemoveSpaces[ i ] == '(' )
        {
            for( int j = i; j < ( int ) strlen( lineToRemoveSpaces ); j++ )
            {
                if( lineToRemoveSpaces[ j ] == ')' )
                {
                    i = j;
                    break;
                }
            }
        }
        if( lineToRemoveSpaces[ i ] == ' ' )
        {
            AdjustLineElements( lineToRemoveSpaces, i );
        }
    }
} // end RemoveSpaces

/**<
Helper function to make removing spaces easier

@pre    None.
@post   The specified string will have all leading characters from the
        position specified moved back one space.
@param  lineToAdjust The line that will be adjusted.
@param  positionToAdjust The position in the string to which we are
        adjusting.
@return void
****************************************************************************/
void MetaDataInfo::AdjustLineElements(  char lineToAdjust[ ], 
                                        int positionToAdjust )
{
    for( int i = positionToAdjust; i < ( int ) strlen( lineToAdjust ); i++ )
    {
        lineToAdjust[ i ] = lineToAdjust[ i + 1 ];
    }
} // end AdjustLineElements

/**<
Helper function make dealing with errors easier

@pre    None.
@post   The specified error code will be handled and logged appropriately.
@param  logSpecification The specification code indicating where to log
        error.
@param  errorCode The code that needs to be handled.
@param  logFile The file where the error will be logged if specified to log
        to file.
@return void
****************************************************************************/
void MetaDataInfo::ProcessErrorCode(    char logSpecification, 
                                        char errorCode, 
                                        ofstream& logFile )
{
    char tempMessage[ 30 ] = {'\0'};
    switch( errorCode )
    {
        case 21:
            strcpy( tempMessage, "Error in pasring function.\n" );
            break;
        case 31:
            strcpy( tempMessage, "Error in meta data code.\n" );
            break;
        case 32:
            strcpy( tempMessage, "Error: Meta data code is wrong case or " );
            strcat( tempMessage, "non-alphabetic\n" );
            break;
        case 33:
            strcpy( tempMessage, "Error: Meta data code is missing.\n" );
            break;
        case 41:
            strcpy( tempMessage, "Error: Error in meta data descriptor.\n" );
            break;
        case 42:
            strcpy( tempMessage, "Error: Meta data descriptor missing.\n" );
            break;
        case 51:
            strcpy( tempMessage, "Error in cycle value.\n" );
            break;
        case 52:
            strcpy( tempMessage, "Error: Cycle value is missing.\n" );
            break;
        case 53:
            strcpy( tempMessage, "Error: Cycle value is negative.\n" );
            break;
        case 54:
            strcpy( tempMessage, "Error: Cycle valye is not numeric.\n" );
            break;
        case 61:
            strcpy( tempMessage, "Error in pasring function.\n" );
            break;
        default:
            strcpy( tempMessage, "Error: An unknown error occured.\n" );
            break;
    }

    LogOutput( logSpecification, tempMessage, logFile );
}

/**<
Helper function to make logging messages easier

@pre    None.
@post   The message to be logged will be logged to the appropriate
        destinations.
@param  logSpecification The specification code indicating where to log
        error.
@param  logMessage The message that needs to be logged.
@param  logFile The file where the error will be logged if specified to log
        to file.
@return void
****************************************************************************/
void MetaDataInfo::LogOutput(   char logSpecification, 
                                char* logMessage, 
                                ofstream& logFile )
{
    if( logSpecification == 'M' 
        || logSpecification == 'B' 
        || logSpecification == 'm' )
    {
        cout << logMessage;
    }
    if( logSpecification == 'F' || logSpecification == 'B' )
    {
        logFile << logMessage;
    }
} // end LogOutput

/**<
Helper function to convert from an integer to a string

@pre    None.
@post   The integer value passed into the function will be converted into
        a string representation to make logging easier.
@param  inputValue The integer value that will be converted to a string.
@param  ouputString The string where the converted value will be stored.
@param  base The base to convert to. i.e. 10 = decimal, 16 = hexadecimal.
@return void
****************************************************************************/ 
char* MetaDataInfo::itoa( int inputValue, char* outputString, int base )
{
    int i = 0;
    bool isNegative = false;

    if( inputValue == 0 )
    {
        outputString[ i++ ] = '0';
        outputString[ i ] = '\0';
    }
    else if( inputValue < 0 )
    {
        isNegative = true;
        inputValue *= -1;
    }

    while( inputValue != 0 )
    {
        int temp = inputValue % base;
        outputString[ i++ ] = ( temp > 9 )? ( temp - 10 ) + 'a' : temp + '0';
        inputValue /= base;
    }

    if( base == 16 )
    {
    	while( i < 8 )
    	{
    		outputString[ i++ ] = '0';
    	}
    	outputString[ i++ ] = 'x';
    	outputString[ i++ ] = '0';
    }

    if( isNegative )
    {
        outputString[ i++ ] = '-';
    }

    outputString[ i-- ] = '\0';

    ReverseString( outputString, i );

    return outputString;
} // end itoa

/**<
Helper function to make converting from int to string easier. 

@pre    None.
@post   The string passed in will be reversed.
@param  string The string to be reversed.
@param  size The size of the string passed in.
@return void
****************************************************************************/ 
void MetaDataInfo::ReverseString( char* string, int size )
{
    char buffer;
    for( int i = size; i > size/2; i-- )
    {
        buffer = string[ i ];
        string[ i ] = string[ size - i ];
        string[ size - i ] = buffer;
    }
} // end ReverseString
void MetaDataInfo::LogTime( 	char logSpecification, 
                        		double time,
                        		ofstream& logFile )
{
    if( logSpecification == 'M' 
        || logSpecification == 'B' 
        || logSpecification == 'm' )
    {
        cout << time;
    }
    if( logSpecification == 'F' || logSpecification == 'B' )
    {
        logFile << time;
    }
}
