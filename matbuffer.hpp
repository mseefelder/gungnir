#include <opencv/cv.hpp>

struct MatBuffer
{
	cv::Mat* data;
	cv::Mat temp;
	int size;
	int last;
	int read;
	int write;
	bool writeLoop;

	MatBuffer(int s)
	{
		size = s;
		data = new cv::Mat[size];
		read = 0;
		write = 0;
		writeLoop = false;
	}
	~MatBuffer()
	{
		delete [] data;
	}

	bool pop(cv::Mat &frame)
	{
		if (write == read && !writeLoop)
		{	
			return false;
		}
		else
		{
			frame = data[read];
			read++;
			if(!(read<size))
			{
				read=0; 
				writeLoop=false;
			}
			return true;
		}
	}

	bool push(cv::VideoCapture* camera)
	{
		if(writeLoop && read==write)
			return false;
		camera->read(temp);
		if (temp.empty())
		{
			std::cout<<"Empty image"<<std::endl;
			return false;
		}
		else
		{
			//imwrite("test.png", temp);
			data[write] = temp;
		}
		write++;
		if(!(write<size))
		{
			write=0; 
			writeLoop=true;
		}
		return true;
	}

	bool push()
	{
		if(writeLoop && read==write)
			return false;

		if (temp.empty())
		{
			std::cout<<"Empty image"<<std::endl;
			return false;
		}
		else
		{
			data[write] = temp;
		}
		write++;
		if(!(write<size))
		{
			write=0; 
			writeLoop=true;
		}
		return true;
	}

	bool push(cv::Mat &frame)
	{
		if(writeLoop && read==write)
			return false;
		
		if (frame.empty())
		{
			std::cout<<"Empty image"<<std::endl;
			return false;
		}
		else
		{
			data[write] = frame;
		}
		write++;
		if(!(write<size))
		{
			write=0; 
			writeLoop=true;
		}
		return true;
	}

	cv::Mat* getTempPointer()
	{
		return &temp;
	}

	
};