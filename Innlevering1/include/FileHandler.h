/********************************************************************
    created:    10:2:2013   17:45
    filename:   FileHandler.h
    author:     Kristian Skarseth
    
    purpose:    Provide functionality to show contents of the model folders
				and allow for loading new models using input to the console.
*********************************************************************/
#ifndef FileHandler_h__
#define FileHandler_h__

#include <boost/filesystem.hpp>
#include <regex>
#include <string>
#include <iostream>
#include <iomanip>
using namespace boost::filesystem;
#include "GameManager.h"
class GameManager;

class FileHandler
{
public:
    FileHandler();
    ~FileHandler();

	/*
	* Initializes the param path as the filehandlers directory.
	* If the string is a successfull path, it's content will be
	* written out to the console.
	*/
	void InitFileHandler(std::string dirPath, GameManager* gameManager);

	/*
	* Writes the help dialog in the console window
	*/
	void DisplayHelp();

	/*
	* Writes the content of the model directory to console
	*/
	void DisplayDirContent();

	void EnterConsoleMode();

protected:

private:
	std::string helpString;

	path curDir, baseDir;

	GameManager* gameManager;

	//Handles the input string from the console. Returns true if we should
	//quit console mode, true if console mode should stop.
	bool ProcessInput(std::string input);

};

#endif // FileHandler_h__