#include "DirectoryBrowser.h"


DirectoryBrowser::DirectoryBrowser()
{
	helpString = "---- Help ----\n";
	helpString += "Press Enter to go to console-mode.\n";
	helpString += "When in console-mode, the console awaits input, and\n";
	helpString += "the modelViewer itself is not receiving input.\n";
	helpString += "To exit console-mode, type exit and press enter, or load a new model.\n";
	helpString += "\nTo load a file, type 'load filename.ext'. The file must be in the current dir\n";
	helpString += "To display directory content, type 'dir'\n\n";
	helpString += "To enter a dir inside current dir, type 'cd' followed by the dir name. Example:\n";
	helpString += "\tcd somedir\n\n";
	helpString += "To jump out one dir, simply type 'cd'.\n";
	helpString += "You cannot go further out than the starting dir.\n";

}

DirectoryBrowser::~DirectoryBrowser()
{
}

void DirectoryBrowser::DisplayHelp()
{
	std::cout << std::endl;
	std::cout << helpString << std::endl;
	std::cout << std::endl;
}

void DirectoryBrowser::DisplayDirContent()
{
	std::cout << "Directory content: " << std::endl;
	directory_iterator end;

	//Wrapping the displaying of content in a try block, just in case
	//some bad person were to have deleted our directory while we were 
	//here playing with the model viewer
	try
	{
		if(exists(curDir))
		{
			//Looping trough the contents of the current directory
			for(directory_iterator iter(curDir); iter != end; ++iter)
			{
				path p = *iter;
				//If what the iterator points to is a regular file, it's name 
				//and file size is written to console
				if(is_regular_file(*iter))
				{
					float size = static_cast<float>(file_size(*iter));
					size /= 1000.0f;
					if(size > 1000.0f)
					{
						size /= 1000;
						std::cout << p.string() << "\t\t" << size << "MB" << std::endl;
					}
					else
						std::cout << p.string() << "\t\t" << size << "KB" << std::endl;
				}
				//If the iterator points to a sub-directory, the folder name is written to console
				else if(is_directory(*iter))
					std::cout << p.string() << "\\" << "\t\t" << "folder" << std::endl;
			}
		}
	}
	catch (const filesystem_error& ex)
	{
		std::cout << ex.what() << std::endl;
	}
}

void DirectoryBrowser::InitFileHandler( std::string dirPath, GameManager* gameManager )
{
	this->gameManager = gameManager;
	this->curDir = this->baseDir = dirPath;

	try
	{
		//Confirmin that the path is an actual path, and writing it's content if it is.
		if (exists(dirPath))
		{
			if (is_directory(dirPath))
				DisplayDirContent();
			else
				std::cout << dirPath << " is no directory.";
		}
		else
			std::cout << dirPath << " does not exist\n";
	}
	catch (const filesystem_error& ex)
	{
		std::cout << ex.what() << std::endl;
	}
}

void DirectoryBrowser::EnterConsoleMode()
{
	std::cout << "\n\nEntered console-mode. Enter a command, or type 'help'." << std::endl;
	bool consoleMode = true;

	//Processing input from the console while consoleMode is true.
	//This will be the case until exit is typed by user, or a model is loaded.
	while(consoleMode)
	{
		std::cout << "\\>";
		std::string input;
		std::getline(std::cin, input);
		if(std::regex_match(input, std::regex("(exit)")))
		{
			consoleMode = false;
			break;
		}
		consoleMode = !ProcessInput(input);
		std::cout << std::endl;
	}
	std::cout << "\n\nExited console-mode. You may resume using the model viewer." << std::endl;
}

bool DirectoryBrowser::ProcessInput( std::string input )
{
	if(input == "dir")
	{
		DisplayDirContent();
		return false;
	}
	
	//If input matches 'load', followed by something, we try to use the following part
	//as a filename in the current directory. If it's a name of a file in the directory,
	//we tell the gameManager to load the file.
	std::string loadMatch = input.substr(0, 5);
	if(loadMatch == "load ")
	{
		std::string fileName = input.substr(5, input.size()-5);
		path filePath = curDir.string() + "/" + fileName;
		if(is_regular_file(filePath))
		{
			std::cout <<"Loading file: " << fileName << "..." << std::endl;
			gameManager->LoadModel(filePath.string());
			return true;
		}
		else
		{
			std::cout << fileName << " is not a valid filename in directory " << curDir.string() << std::endl;
			return false;
		}
	}

	//if input matched 'cd' we try to move out one directory
	if(input == "cd")
	{
		if(curDir == baseDir)
			std::cout << "cannot go further out, already in base directory" << std::endl;
		else
		{
			curDir = curDir.branch_path();
			std::cout << "Exiting current directory" << std::endl;
		}
		return false;
	}
	//<if input matches 'cd', followed by something else, we try to use the following part
	//as a new directory path. If it's a successful path, we enter the new directory.
	else if(input.substr(0, 3) == "cd ")
	{
		std::string dirName = input.substr(3, input.size()-3);
		path dirPath = curDir.string() + "/" + dirName;
		if(is_directory(dirPath))
		{
			std::cout <<"Entering directory: " << dirName << std::endl;
			curDir = dirPath;
		}
		else
		{
			std::cout << dirName << " is not a valid directory inside directory " << curDir.string() << std::endl;
		}
		return false;
	}

	std::cout << input << " is an invalid command" << std::endl;
	return false;
}
