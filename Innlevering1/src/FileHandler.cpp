#include "FileHandler.h"


FileHandler::FileHandler()
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

FileHandler::~FileHandler()
{
}

void FileHandler::DisplayHelp()
{
	std::cout << std::endl;
	std::cout << helpString << std::endl;
	std::cout << std::endl;
}

void FileHandler::DisplayDirContent()
{
	std::cout << "Directory content: " << std::endl;
	directory_iterator end;
	for(directory_iterator iter(curDir); iter != end; ++iter)
	{
		path p = *iter;
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
		else if(is_directory(*iter))
		{
			std::cout << p.string() << "\\" << "\t\t" << "folder" << std::endl;
		}
	}
}

void FileHandler::InitFileHandler( std::string dirPath, GameManager* gameManager )
{
	this->gameManager = gameManager;
	this->curDir = this->baseDir = dirPath;

	//The following try-block is part of a boost::filesystem example.
	try
	{
		if (exists(dirPath))    // does p actually exist?
		{
			if (is_regular_file(dirPath))        // is p a regular file?
				std::cout << dirPath << " size is " << file_size(dirPath) << '\n';

			else if (is_directory(dirPath))      // is p a directory?
			{
				DisplayDirContent();
			}
			else
				std::cout << dirPath << " exists, but is neither a regular file nor a directory\n";
		}
		else
			std::cout << dirPath << " does not exist\n";
	}

	catch (const filesystem_error& ex)
	{
		std::cout << ex.what() << '\n';
	}
}

void FileHandler::EnterConsoleMode()
{
	std::cout << "\n\nEntered console-mode. Enter a command, or type 'help'." << std::endl;
	bool consoleMode = true;
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

bool FileHandler::ProcessInput( std::string input )
{
	if(input == "dir")
	{
		DisplayDirContent();
		return false;
	}
	
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
