#include <iostream>
#include <cstring>
#include "ConfigFileInput.hh"
#include "MetaDataInfo.hh"
#include "PCB.hh"

using namespace std;

int main( int argv, char* argc[ ] )
{
	PCB state;
	state.processState = 1;

    ConfigFileInput ConfigFileTest( argc[ 1 ] );

    MetaDataInfo MetaDataFileTest( 	ConfigFileTest.GetFilePath( ), ConfigFileTest.GetCPUSchedulingCode( ) );

    MetaDataFileTest.ProcessData( ConfigFileTest, state );

	return 0;
}
