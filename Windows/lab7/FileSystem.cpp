#include "FileSystem.h"
#include <conio.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using namespace filesystem;

const char FileSystem::SOURCE_NAME_[] = "disk.bin";

FileSystem::FileSystem()
{
	this->free_space_ = sizeof(Block) * AMOUNT_OF_BLOCKS_;
	SecureZeroMemory(this->fmdt_, this->MAX_AMOUNT_OF_FILES_ * sizeof(FileMetaDataTableField));
	this->source_file_.open(this->SOURCE_NAME_, ios::binary | ios::in | ios::out | ios::ate);

	if (!this->source_file_.is_open())
	{
		this->source_file_.open(this->SOURCE_NAME_, ios::binary | ios::out | ios::ate);

		const int empty_file_size = this->AMOUNT_OF_BLOCKS_ * sizeof(Block) + this->FMDT_SIZE;
		byte set_arry[empty_file_size];
		SecureZeroMemory(set_arry, empty_file_size);
		this->source_file_.write(set_arry, empty_file_size);

		this->source_file_.close();
		this->source_file_.open(this->SOURCE_NAME_, ios::binary | ios::in | ios::out | ios::ate);
	}
	else
	{
		this->source_file_.seekg(0);
		this->source_file_.read((byte*)this->fmdt_, sizeof(FileMetaDataTableField) * this->MAX_AMOUNT_OF_FILES_);
	}
}

FileSystem::~FileSystem()
{
	this->source_file_.close();
}

void FileSystem::Help()
{
	cout << endl;
	cout << "Available commands" << endl;
	cout << "----------------------------------------" << endl;
	cout << "dir - show all files" << endl;
	cout << "addfile - adding a new file" << endl;
	cout << "remove - removing a component" << endl;
	cout << "reset - reset disk" << endl;
	cout << "clear - clear screen" << endl;
	cout << "open - open file" << endl;
	cout << "exit - exit the programm" << endl;
	cout << "----------------------------------------" << endl;
	cout << endl;
}

void FileSystem::ResetDisk()
{
	this->free_space_ = sizeof(Block) * AMOUNT_OF_BLOCKS_;
	SecureZeroMemory(this->fmdt_, this->MAX_AMOUNT_OF_FILES_ * sizeof(FileMetaDataTableField));

	this->source_file_.close();
	this->source_file_.open(this->SOURCE_NAME_, ios::binary | ios::out | ios::trunc);

	const int empty_file_size = this->AMOUNT_OF_BLOCKS_ * sizeof(Block) + this->FMDT_SIZE;
	byte set_arry[empty_file_size];
	SecureZeroMemory(set_arry, empty_file_size);
	this->source_file_.write(set_arry, empty_file_size);

	this->source_file_.close();
	this->source_file_.open(this->SOURCE_NAME_, ios::binary | ios::in | ios::out | ios::ate);

	cout << endl << endl << "The disk was successfully reseted..." << endl << endl;
}

void FileSystem::ShowFiles()
{
	cout << endl << "All files: " << endl << endl;

	for (int i = 0; i < this->MAX_AMOUNT_OF_FILES_; i++)
	{
		if (this->fmdt_[i].begining_offset != 0)
		{
			cout << '-' << this->fmdt_[i].name << endl;
		}
	}
	
	cout << endl;
}

void FileSystem::CreateNewFile()
{
	cout << "Please, enter the name of a new file" << endl;

	string new_file_name;
	cin >> new_file_name;

	for (int i = 0; i < this->MAX_AMOUNT_OF_FILES_; i++)
	{
		if (strcmp(this->fmdt_[i].name, new_file_name.c_str()) == 0)
		{
			cout << endl << "File name is already in use, please repeat..." << endl;
			return;
		}
	}

	for (int i = 0; i < this->MAX_AMOUNT_OF_FILES_; i++)
	{
		if (this->fmdt_[i].begining_offset == 0)
		{
			this->fmdt_[i].begining_offset = this->FindFreeBlock(false);
			if (this->fmdt_[i].begining_offset == 0)
				break;

			int copy_size = this->MAX_FILE_NAME_LENGTH_ < new_file_name.length() ? this->MAX_FILE_NAME_LENGTH_ : new_file_name.length();
			CopyString(this->fmdt_[i].name, copy_size, new_file_name.c_str());
			
			this->SaveFMDT();

			Block temp_block;
			temp_block.busy_flag = 1;
			this->SaveBlock(temp_block, this->fmdt_[i].begining_offset);

			return;
		}
	}
	cout << endl << endl << "Max amount of files is reached! Free some space, to perform this operation..." << endl << endl;
}

void FileSystem::SaveFMDT()
{
	this->source_file_.seekp(0);
	this->source_file_.write((byte*)this->fmdt_, sizeof(FileMetaDataTableField) * this->MAX_AMOUNT_OF_FILES_);
	this->source_file_.flush();
}

void FileSystem::SaveBlock(Block block, int offset)
{
	this->source_file_.seekp(offset);
	this->source_file_.write((byte*)&block, sizeof(Block));
	this->source_file_.clear();
	this->source_file_.flush();
	this->source_file_.clear();
}

void FileSystem::ClearBlock(Block &block)
{
	block.next_offset = 0;
	block.busy_flag = 0;
	SecureZeroMemory(block.data, this->BLOCK_DATA_SIZE);
}

FileSystem::Block FileSystem::ReadBlock(int offset)
{
	Block temp_block;
	this->source_file_.seekg(offset);
	this->source_file_.read((byte*)&temp_block, sizeof(Block));
	this->source_file_.clear();
	return temp_block;
}

int FileSystem::FindFreeBlock(bool next)
{
	byte current_flag = 0;
	int offset = this->FIRST_BLOCK_OFFSET_;
	for (int i = 0; i < this->AMOUNT_OF_BLOCKS_; i++)
	{
		this->source_file_.seekg(offset);
		source_file_.read(&current_flag, sizeof(byte));

		if (current_flag == 0)
		{
			if (next)
			{
				next = false;
				continue;
			}
			return offset;
		}
		offset += sizeof(Block);
	}
	return 0;
}

void FileSystem::CopyString(char* destinantion, int destination_max_size, const char* source)
{
	for (int i = 0; i < destination_max_size && source[i] != '\0'; i++)
	{
		destinantion[i] = source[i];
	}
}

void FileSystem::RemoveFile()
{
	cout << "Please, enter the name of a file to delete" << endl;

	cin.clear();
	string deleting_file;
	cin >> deleting_file;

	for (int i = 0; i < this->MAX_AMOUNT_OF_FILES_; i++)
	{
		if (strcmp(this->fmdt_[i].name, deleting_file.c_str()) == 0)
		{
			int offset = this->fmdt_[i].begining_offset;
			while (true)
			{
				Block temp_block = this->ReadBlock(offset);

				int next_offset = temp_block.next_offset;

				this->ClearBlock(temp_block);

				this->SaveBlock(temp_block, offset);

				if (next_offset == 0)
					break;
				else
					offset = next_offset;
			}
			this->fmdt_[i].begining_offset = 0;
			SecureZeroMemory(this->fmdt_[i].name, this->MAX_FILE_NAME_LENGTH_);

			this->SaveFMDT();

			return;
		}
	}
	cout << endl << "Incorrect file name, please repeat..." << endl;
}


string FileSystem::FileToBuffer(int begining_offset)
{
	string result_str;
	int offset = begining_offset;
	while (true)
	{
		Block temp_block = this->ReadBlock(offset);

		char temp_buffer[this->BLOCK_DATA_SIZE + 1] = { 0 };
		this->CopyString(temp_buffer, this->BLOCK_DATA_SIZE, temp_block.data);

		result_str.append(temp_buffer);

		if (temp_block.next_offset == 0)
			return result_str;
		else
			offset = temp_block.next_offset;
	}
}

void FileSystem::OpenFile()
{
	cout << "Please, enter the name of a file" << endl;

	string current_file;
	cin >> current_file;

	for (int i = 0; i < this->MAX_AMOUNT_OF_FILES_; i++)
	{
		if (strcmp(this->fmdt_[i].name, current_file.c_str()) == 0)
		{
			string fileBuffer = this->FileToBuffer(this->fmdt_[i].begining_offset);

			string processed;
			processed = this->ProcessFileBuffer(fileBuffer);

			if (processed.length() == 0)
			{
				int offset = this->fmdt_[i].begining_offset;
				while (true)
				{
					Block temp_block = this->ReadBlock(offset);

					int next_offset = temp_block.next_offset;

					this->ClearBlock(temp_block);

					this->SaveBlock(temp_block, offset);

					if (next_offset == 0)
						break;
					else
						offset = next_offset;
				}

				Block firstFileBlock;
				firstFileBlock.busy_flag = 1;
				this->SaveBlock(firstFileBlock, this->fmdt_[i].begining_offset);

				return;
			}

			this->SaveFile(processed, this->fmdt_[i].begining_offset);
			return;
		}
	}

	cout << endl << "Incorrect file name, please repeat..." << endl;
	return;
}

void FileSystem::SaveFile(string processed, int begining_offset)
{
	int unprocessed_length = processed.length();
	int offset = begining_offset;
	while (true)
	{
		int min_size = unprocessed_length < this->BLOCK_DATA_SIZE ? unprocessed_length : this->BLOCK_DATA_SIZE;

		Block temp_block = this->ReadBlock(offset);
		SecureZeroMemory(temp_block.data, this->BLOCK_DATA_SIZE);
		temp_block.busy_flag = 1;

		this->CopyString(temp_block.data, min_size, processed.c_str());

		this->SaveBlock(temp_block, offset);

		unprocessed_length -= min_size;
		processed.erase(0, min_size);

		if (unprocessed_length == 0)
		{
			if (temp_block.next_offset != 0)
			{
				int clear_offset = temp_block.next_offset;
				temp_block.next_offset = 0;
				this->SaveBlock(temp_block, offset);
				while (true)
				{
					Block clear_block = this->ReadBlock(clear_offset);

					int old_offset = clear_offset;
					clear_offset = clear_block.next_offset;

					this->ClearBlock(clear_block);
					this->SaveBlock(clear_block, old_offset);

					if (clear_offset == 0)
						break;
				}
			}
			else return;
		}

		if (temp_block.next_offset == 0)
		{
			if (unprocessed_length != 0)
			{
				temp_block.next_offset = this->FindFreeBlock(false);

				if (temp_block.next_offset == 0)
				{
					cout << endl << endl << "Not enough space on disk! Some information was lost!" << endl << endl;
					return;
				}

				this->SaveBlock(temp_block, offset);
			}
			else
			{
				return;
			}
		}

		offset = temp_block.next_offset;
	}
}

string FileSystem::ProcessFileBuffer(string fileBuffer)
{
	string processed = fileBuffer;

	while (true)
	{
		system("cls");

		cout << processed << endl << endl;

		cout << "Choose an option:" << endl;
		cout << "1. Clear file" << endl;
		cout << "2. Remove from position" << endl;
		cout << "3. Add to position" << endl;
		cout << "0. Close file and save changes" << endl;

		switch (_getch())
		{
		case '0':
		{
			return processed;
		}
		case '1':
		{
			processed.clear();
			break;
		}
		case '2':
		{
			int delete_position = -1;
			int delete_size = -1;

			while (delete_position < 0 || delete_size < 0 || ((delete_size + delete_position) > processed.length() + 1))
			{
				cout << "Please, enter the position, where to start deleting..." << endl;
				cin >> delete_position;

				cout << "Please, enter the size of a fragment..." << endl;
				cin >> delete_size;
			}

			processed.erase(delete_position, delete_size);

			break;
		}
		case '3':
		{
			cout << "Please, select option:" << endl;
			cout << "1. Append" << endl;
			cout << "2. From position" << endl;

			char symbol = 0;
			while (symbol != '1' && symbol != '2')
			{
				symbol = _getch();
			}


			string additional;

			cout << "Continue..." << endl;

			cin.ignore();
			cin.clear();

			cout << "Please, enter the text" << endl;
			getline(cin, additional);

			if (symbol == '1')
			{
				processed.append(additional);
			}
			else
			{
				int adding_position = -1;
				while (adding_position < 0 ||( adding_position > processed.length() - 1))
				{
					cout << "Please, enter the position, where to start adding text..." << endl;
					cin >> adding_position;
				}

				processed.insert(adding_position, additional);
			}
			break;
		}
		}
	}
}
