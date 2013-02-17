/********************************************************************
    created:    10:2:2013   17:45
    filename:   DirectoryBrowser.h
    author:     Kristian Skarseth
    
    purpose:    Provide functionality to show contents of the model folders,
				and perform loading of new models using input to the console.
*********************************************************************/
#ifndef DirecotryBrowser_h__
#define DirecotryBrowser_h__

#include <boost/filesystem.hpp>
#include <regex>
#include <string>
#include <iostream>
#include <iomanip>
#include <stack>

using namespace boost::filesystem;
#include "GameManager.h"
#include "TextRenderer.h"

class GameManager;

struct DirectoryContent
{

};

class DirectoryBrowser
{
public:
    DirectoryBrowser();
    ~DirectoryBrowser();
	
	bool ToggleDirectoryBrowser(){ showDirBrowser = !showDirBrowser; return showDirBrowser;}

	void RenderDirectoryBrowser();

	/*
	* Initializes the param path as the filehandlers directory.
	* If the string is a successful path, it's content will be
	* written out to the console.
	*/
	void Init(std::string dirPath, GameManager* gameManager,
			  std::shared_ptr<GLUtils::Program>textProgram);
			  //unsigned int windowWidth, unsigned int windowHeight);

	/*
	* Writes the content of the model directory to console
	*/
	void DisplayDirContent();

	/*
	* Start console mode. Will route all input updates to this class until
	* console mode is ended by user typing exit, or a new model being loaded.
	*/
	void EnterConsoleMode();

protected:

private:
	bool showDirBrowser;

	std::stack<std::vector<Text>> DirStack;
	
	void PushNewDirectory(path newDir);
	
	void PopDirectory();

	path curDir, baseDir; //< boost directory paths

	//pointer to the GameManager object, allowing for files
	//to be loaded trough the propper channels
	GameManager* gameManager;

	std::shared_ptr<TextRenderer> textRenderer;

	unsigned int WW();
	unsigned int WH();
	
	float PaddingY;
};

#endif // DirecotryBrowser_h__