/**<
@file ConfigFileInputNode.hh
@author Eugene Nelson
@breif  The header file for the ConfigFileInputNode class
@version    1.0 Eugene Nelson 
            Originally Developed ( 9 - 18 - 17 )
*/
/////////////////////////////////////////////////////////////////////////////
// Precompiler Directives
/////////////////////////////////////////////////////////////////////////////
#ifndef namespace_config_file_input_hh
#define namespace_config_file_input_hh

/////////////////////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////////////////////
#define FILE_NAME_MAX_LENGTH 30
#define STR_MAX_LENGTH 300
#define LOG_FILE_NAME_MAX_LENGTH 30

/////////////////////////////////////////////////////////////////////////////
// Header Files
/////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdlib.h>
#include <list>

/////////////////////////////////////////////////////////////////////////////
// Namespace Declaration
/////////////////////////////////////////////////////////////////////////////
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// Struct Definition
/////////////////////////////////////////////////////////////////////////////
struct ConfigFileInputNode 
{
    char   aProcessName[ STR_MAX_LENGTH ];
    int    aProcessValue;
    int    aQuantity;
    
    ConfigFileInputNode( )
    {
        aProcessValue = -1;
        aQuantity = 1;
    }
    ConfigFileInputNode( const ConfigFileInputNode& copyNode )
    {
        strcpy( aProcessName, copyNode.aProcessName );
        aProcessValue = copyNode.aProcessValue;
        aQuantity = copyNode.aQuantity;
    }
    ~ConfigFileInputNode( )
    {
        ;
    }
};

/////////////////////////////////////////////////////////////////////////////
// Class Definitions
/////////////////////////////////////////////////////////////////////////////
class ConfigFileInput
{
    public:
        ConfigFileInput( char* fileName );
        ConfigFileInput( const ConfigFileInput& copyInput );
        ~ConfigFileInput( );

        int     GetNumberOfProcesses( );
        int     GetProcessValue( const char processName[ ] );
        int     GetProcessNumber( const char processName[ ] );
        char*   GetProcessName( const int position );
        int     GetProcessQuantity( const char processName[ ] );
        ConfigFileInputNode* GetProcess( const char processName[ ] );

        char    GetLogOutputSpecification( );
        char*   GetFilePath( );
        char*   GetLogFilePath( );
        char*   GetCPUSchedulingCode( );
        int     GetProcessorQuantumNum( );
        unsigned int GetSystemMemory( );
        unsigned int GetMemoryBlockSize( );
    protected:
        bool ParseLine( char lineToParse[ ] );
        void RemoveSpaces( char lineToRemoveSpaces[ ] );
        void AdjustLineElements( char lineToAdjust[ ], int positionToAdjust );
    private:
        char aFilePath[ FILE_NAME_MAX_LENGTH ];
        char aLogOutputSpecification;
        char aLogFilePath[ LOG_FILE_NAME_MAX_LENGTH ];
        char aCPUScheudlingCode[ 5 ];

        int aProcessorQuantumNum;       

        unsigned int aMemoryBlockSize;
        unsigned int aSystemMemory;
        list<ConfigFileInputNode> aListOfProcesses;
};

#endif
