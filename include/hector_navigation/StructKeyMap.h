/// h file for strucet key map container

#ifndef STRUCTKEYMAP_H
#define STRUCTKEYMAP_H



using namespace std;

//according to (x, y, z) cartesian coordinate to find center value

struct position 
{
   double x;
   double y;
   double z;

   bool operator <(const position& other) const
   {
        if (x < other.x)
	{
	 return true;
	}  
  
	else if (x == other.x)  
	{
           if (y < other.y)
	    {
	      return true;
	    } 
	    else if (y == other.y)
	    {
              return z < other.z;
            }
         }

        return false;
   }
};

//according to (theta, phi, distance) sphere coordinate to find center value
struct sphere
{

  double Theta;
  double Phi;
  double R;

   bool operator <(const sphere& other) const
   {
        if (R < other.R)
	{
	 return true;
	}  
  
	else if (R == other.R)  
	{
           if (Theta < other.Theta)
	    {
	      return true;
	    } 
	    else if (Theta == other.Theta)
	    {
              return Phi < other.Phi;
            }
         }

        return false; 
   }
};

//according to (Theta, Phi) spherical coordinate to find mv value
struct Vector
{

  double THETA;
  double PHI;
 

   bool operator <(const Vector& other) const
   {
        if (THETA < other.THETA)
	{
	   return true;
	}  
  
	else if (THETA  == other.THETA )  
	{         
           return PHI < other.PHI;
            
         }

        return false;
   }
};
#endif

