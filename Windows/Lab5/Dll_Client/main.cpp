// SPO_DYNAMIC_DLL_LINKING.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"  
#include <iostream>
#include "IAsyncReadWrite.h"

using namespace std;

typedef IAsyncReadWrite* (*__cdecl  AsyncReadWriteFactoryMethod)();

int main()
{
	BOOL freeResult, runTimeLinkSuccess = FALSE;
	HINSTANCE dllHandle = NULL;
	AsyncReadWriteFactoryMethod AsyncReadWriteFactory = NULL;

	//Load the dll and keep the handle to it
	dllHandle = LoadLibrary(L"AsyncInterfaceImplementation.dll");

	// If the handle is valid, try to get the function address. 
	if (NULL != dllHandle)
	{
		//Get pointer to our function using GetProcAddress:
		AsyncReadWriteFactory = (AsyncReadWriteFactoryMethod)GetProcAddress(dllHandle,
			"AsyncReadWriteFactory");

		// If the function address is valid, call the function. 
		if (runTimeLinkSuccess = (NULL != AsyncReadWriteFactory))
		{
			IAsyncReadWrite* async_read_write_pointer = AsyncReadWriteFactory();

			cout << "Starting processing..." << endl;

			async_read_write_pointer->ConcatFiles("read_files", "output.txt");

			// Is necessary to use to release class object (just like using delete to free dynamicly allocated memory)
			async_read_write_pointer->Release();

			cout << "The task was executed..." << endl;
			cout << "Press any key to continue..." << endl;
		}

		//Free the library:
		freeResult = FreeLibrary(dllHandle);
	}

	//If unable to call the DLL function, use an alternative. 
	if (!runTimeLinkSuccess)
	{
		cout << "Unable to call dll function" << endl;
		if (NULL != dllHandle)
		{
			freeResult = FreeLibrary(dllHandle);
		}
	}

	cin.ignore();

	return 0;
}
