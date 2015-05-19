#include <exception>
using namespace std;

class cameraException: public exception
{
  virtual const char* what() const throw()
  {
    return "No working camera found!";
  }
};