//Header Stuff:
typedef double reals;
const reals One=1.0,Two=2.0,Three=3.0,Four=4.0,Five=5.0,Six=6.0,Ten=10.0;

/************************************************************************
      DECLARATION OF THE CLASS CIRCLE
************************************************************************/
// Class for Circle
// A circle has 7 fields: 
//     a, b, r (of type reals), the circle parameters
//     s (of type reals), the estimate of sigma (standard deviation)
//     g (of type reals), the norm of the gradient of the objective function
//     i and j (of type int), the iteration counters (outer and inner, respectively)

class Circle
{
public:

  // The fields of a Circle
  reals a, b, r, s, g, Gx, Gy;
  int i, j;

  // constructors
  Circle();
  Circle(reals aa, reals bb, reals rr);

  // no destructor we didn't allocate memory by hand.
};


/************************************************************************
      BODY OF THE MEMBER ROUTINES
************************************************************************/
// Default constructor

Circle::Circle()
{
  a=0.; b=0.; r=1.; s=0.; i=0; j=0;
}

// Constructor with assignment of the circle parameters only

Circle::Circle(reals aa, reals bb, reals rr)
{
  a=aa; b=bb; r=rr;
}

/************************************************************************
      DECLARATION OF THE CLASS DATA
************************************************************************/
// Class for Data
// A data has 5 fields: 
//       n (of type int), the number of data points 
//       X and Y (arrays of type reals), arrays of x- and y-coordinates
//       meanX and meanY (of type reals), coordinates of the centroid (x and y sample means)

class Data
{
public:

  int n;
  reals *X;   //space is allocated in the constructors
  reals *Y;   //space is allocated in the constructors
  reals meanX, meanY;

  // constructors
  Data();
  Data(int N);
  Data(int N, reals X[], reals Y[]);

  // routines
  void means(void);
  void center(void);
  void scale(void);
  void print(void);

  // destructors
  ~Data();
};


/************************************************************************
      BODY OF THE MEMBER ROUTINES
************************************************************************/
// Default constructor
Data::Data()
{
  n=0;
  X = new reals[n];
  Y = new reals[n];
  for (int i=0; i<n; i++)
  {
    X[i]=0.;
    Y[i]=0.;
  }
}

// Constructor with assignment of the field N
Data::Data(int N)
{
  n=N;
  X = new reals[n];
  Y = new reals[n];

  for (int i=0; i<n; i++)
  {
    X[i]=0.;
    Y[i]=0.;
  }
}

// Constructor with assignment of each field
Data::Data(int N, reals dataX[], reals dataY[])
{
  n=N;
  X = new reals[n];
  Y = new reals[n];

  for (int i=0; i<n; i++)
  {
    X[i]=dataX[i];
    Y[i]=dataY[i];
  }
}

// Routine that computes the x- and y- sample means (the coordinates of the centeroid)

void Data::means(void)
{
  meanX=0.; meanY=0.;
  
  for (int i=0; i<n; i++)
  {
    meanX += X[i];
    meanY += Y[i];
  }
  meanX /= n;
  meanY /= n;
}

// Routine that centers the data set (shifts the coordinates to the centeroid)

void Data::center(void)
{
  reals sX=0.,sY=0.;  
  int i;
  
  for (i=0; i<n; i++)
  {
    sX += X[i];
    sY += Y[i];
  }
  sX /= n;
  sY /= n;
  
  for (i=0; i<n; i++)
  {
    X[i] -= sX;
    Y[i] -= sY;
  }
  meanX = 0.;
  meanY = 0.;
}

// Routine that scales the coordinates (makes them of order one)

void Data::scale(void)
{
  reals sXX=0.,sYY=0.,scaling;  
  int i;
  
  for (i=0; i<n; i++)
  {
    sXX += X[i]*X[i];
    sYY += Y[i]*Y[i];
  }
  scaling = sqrt((sXX+sYY)/n/Two);
  
  for (i=0; i<n; i++)
  {
    X[i] /= scaling;
    Y[i] /= scaling;
  }
}

// Destructor
Data::~Data()
{
  delete[] X;
  delete[] Y;
}

template<typename T>
inline T SQR(T t) { return t*t; };

//****************** Sigma ************************************
//
//   estimate of Sigma = square root of RSS divided by N
//   gives the root-mean-square error of the geometric circle fit

reals Sigma (Data& data, Circle& circle)
{
    reals sum=0.,dx,dy;

    for (int i=0; i<data.n; i++)
    {
        dx = data.X[i] - circle.a;
        dy = data.Y[i] - circle.b;
        sum += SQR(sqrt(dx*dx+dy*dy) - circle.r);
    }
    return sqrt(sum/data.n);
}

Circle CircleFitByTaubin (Data& data)
/*  
      Circle fit to a given set of data points (in 2D)
      
      This is an algebraic fit, due to Taubin, based on the journal article
     
      G. Taubin, "Estimation Of Planar Curves, Surfaces And Nonplanar
                  Space Curves Defined By Implicit Equations, With 
                  Applications To Edge And Range Image Segmentation",
                  IEEE Trans. PAMI, Vol. 13, pages 1115-1138, (1991)

      Input:  data     - the class of data (contains the given points):
    
        data.n   - the number of data points
        data.X[] - the array of X-coordinates
        data.Y[] - the array of Y-coordinates
     
     Output:         
               circle - parameters of the fitting circle:
            
         circle.a - the X-coordinate of the center of the fitting circle
         circle.b - the Y-coordinate of the center of the fitting circle
         circle.r - the radius of the fitting circle
         circle.s - the root mean square error (the estimate of sigma)
         circle.j - the total number of iterations
             
     The method is based on the minimization of the function
     
                  sum [(x-a)^2 + (y-b)^2 - R^2]^2
              F = -------------------------------
                      sum [(x-a)^2 + (y-b)^2]
                 
     This method is more balanced than the simple Kasa fit.
        
     It works well whether data points are sampled along an entire circle or
     along a small arc. 
     
     It still has a small bias and its statistical accuracy is slightly
     lower than that of the geometric fit (minimizing geometric distances),
     but slightly higher than that of the very similar Pratt fit. 
     Besides, the Taubin fit is slightly simpler than the Pratt fit
     
     It provides a very good initial guess for a subsequent geometric fit. 
     
       Nikolai Chernov  (September 2012)

*/
{
    int i,iter,IterMAX=99;
    
    reals Xi,Yi,Zi;
    reals Mz,Mxy,Mxx,Myy,Mxz,Myz,Mzz,Cov_xy,Var_z;
    reals A0,A1,A2,A22,A3,A33;
    reals Dy,xnew,x,ynew,y;
    reals DET,Xcenter,Ycenter;
    
    Circle circle;
    
    data.means();   // Compute x- and y- sample means (via a function in the class "data")

//     computing moments 

  Mxx=Myy=Mxy=Mxz=Myz=Mzz=0.;
    
    for (i=0; i<data.n; i++)
    {
        Xi = data.X[i] - data.meanX;   //  centered x-coordinates
        Yi = data.Y[i] - data.meanY;   //  centered y-coordinates
        Zi = Xi*Xi + Yi*Yi;
        
        Mxy += Xi*Yi;
        Mxx += Xi*Xi;
        Myy += Yi*Yi;
        Mxz += Xi*Zi;
        Myz += Yi*Zi;
        Mzz += Zi*Zi;
    }
    Mxx /= data.n;
    Myy /= data.n;
    Mxy /= data.n;
    Mxz /= data.n;
    Myz /= data.n;
    Mzz /= data.n;
    
//      computing coefficients of the characteristic polynomial
    
    Mz = Mxx + Myy;
    Cov_xy = Mxx*Myy - Mxy*Mxy;
    Var_z = Mzz - Mz*Mz;
    A3 = Four*Mz;
    A2 = -Three*Mz*Mz - Mzz;
    A1 = Var_z*Mz + Four*Cov_xy*Mz - Mxz*Mxz - Myz*Myz;
    A0 = Mxz*(Mxz*Myy - Myz*Mxy) + Myz*(Myz*Mxx - Mxz*Mxy) - Var_z*Cov_xy;
    A22 = A2 + A2;
    A33 = A3 + A3 + A3;

//    finding the root of the characteristic polynomial
//    using Newton's method starting at x=0  
//     (it is guaranteed to converge to the right root)
    
    for (x=0.,y=A0,iter=0; iter<IterMAX; iter++)  // usually, 4-6 iterations are enough
    {
          Dy = A1 + x*(A22 + A33*x);
        xnew = x - y/Dy;
        if ((xnew == x)||(!isfinite(xnew))) break;
        ynew = A0 + xnew*(A1 + xnew*(A2 + xnew*A3));
        if (abs(ynew)>=abs(y))  break;
        x = xnew;  y = ynew;
    }
     
//       computing paramters of the fitting circle
    
    DET = x*x - x*Mz + Cov_xy;
    Xcenter = (Mxz*(Myy - x) - Myz*Mxy)/DET/Two;
    Ycenter = (Myz*(Mxx - x) - Mxz*Mxy)/DET/Two;

//       assembling the output

    circle.a = Xcenter + data.meanX;
    circle.b = Ycenter + data.meanY;
    circle.r = sqrt(Xcenter*Xcenter + Ycenter*Ycenter + Mz);
    circle.s = Sigma(data,circle);
    circle.i = 0;
    circle.j = iter;  //  return the number of iterations, too
    
    return circle;
}
/*
void setup() {
  Serial.begin(9600);
  double testx[3] = {-1,1,0,};
  double testy[3] = {0,0,1};
  Data testdata = Data(3,testx,testy);
  Circle testcirc = CircleFitByTaubin(testdata);
  Serial.println(testcirc.a);
  Serial.println  (testcirc.r);
}

void loop() {
int j;
}
*/
