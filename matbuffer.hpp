#include <opencv/cv.hpp>

struct MatBuffer
{
	cv::Mat* data;
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

	bool pop(cv::Mat *frame)
	{
		if (write == read && !writeLoop)
		{	
			return false;
		}
		else
		{
			frame = &data[read];
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
		camera->read(data[write]);
		write++;
		if(!(write<size))
		{
			write=0; 
			writeLoop=true;
		}
		return true;
	}

	
};