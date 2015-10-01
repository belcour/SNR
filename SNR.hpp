// Include STL
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>

// Include TinyEXR
#define TINYEXR_IMPLEMENTATION
#include <tinyexr/tinyexr.h>

using namespace std ;

//#define DEBUG_SNR

float* loadImage(const std::string& name, int& W, int &H) {
   float* img = nullptr;
   const char* err;
   int ret = LoadEXR(&img, &W, &H, name.c_str(), &err);
   return img;
}

int b = 0;
void setBorder(int border)
{
	b = border ;
}

float SNR(const string& image, const string& reference)
{
	int W, H ;
	float *img1, *img2 ;

   img1 = loadImage(reference, W, H);
	if(img1 == nullptr)
	{
		cerr << "Unable to load the first image" << endl ;
		return 0 ;
	}

	int Wtemp, Htemp ;
   img2 = loadImage(image, Wtemp, Htemp);
	if(img2 == nullptr || W != Wtemp || H != Htemp)
	{
		cerr << "Unable to load the first image" << endl ;
		return 0 ;
	}

	float error  = 0.0f ;
	float signal = 0.0f ;
	float smax   = 0.0f ;
	for(int i=b; i<W-b; ++i)
		for(int j=b; j<H-b; ++j)
		{
			error  += pow(img2[(i + W*j)*3+0] - img1[(i + W*j)*3+0], 2) ;
			error  += pow(img2[(i + W*j)*3+1] - img1[(i + W*j)*3+1], 2) ;
			error  += pow(img2[(i + W*j)*3+2] - img1[(i + W*j)*3+2], 2) ;
			signal += pow(img1[(i + W*j)*3+0], 2) ;
			signal += pow(img1[(i + W*j)*3+1], 2) ;
			signal += pow(img1[(i + W*j)*3+2], 2) ;
			smax = fmax(img1[(i+j*W)*3+0], smax) ;
			smax = fmax(img1[(i+j*W)*3+1], smax) ;
			smax = fmax(img1[(i+j*W)*3+2], smax) ;
		}
	error  /= (float)((W-2*b)*(H-2*b)) ;
	signal /= (float)((W-2*b)*(H-2*b)) ;

	const float snr    = signal / error ;
	const float psnr   = smax*smax / error ;
	const float psnrdb = 10 * (log10(smax*smax) - log10(error)) ;
	const float snrdb  = 10 * (log10(signal) - log10(error)) ;

   /*
	cout << "SNR = " << snr << endl ;
	cout << "SNRdb = " <<  snrdb << " db" << endl ;

	cout << "PSNR = " << psnr << endl ;
	cout << "PSNRdb = " <<  psnrdb << " db" << endl ;
	*/
   return snr;
} ;

float RMSE(const string& image, const string& reference)
{
	int W, H ;
	float *img1, *img2 ;
   
   img1 = loadImage(reference, W, H);
	if(img1 == nullptr)
	{
		cerr << "Unable to load the first image" << endl ;
		return 0 ;
	}

	int Wtemp, Htemp ;
   img2 = loadImage(image, Wtemp, Htemp);
	if(img2 == nullptr || W != Wtemp || H != Htemp)
	{
		cerr << "Unable to load the first image" << endl ;
		return 0 ;
	}

#ifdef DEBUG_SNR
	float* temp = new float[W*H] ;
	for(int k=0; k<W*H; ++k)
		temp[k] = 0.0f ;
#endif

	double error_max  = 0.0 ;
	double error_mean = 0.0 ;
	for(int i=0; i<W; ++i)
		for(int j=0; j<H; ++j)
		{
         const int index = (i + W*j)*3;
         if(isnan(img1[index+0]) || isnan(img2[index+0]) ||
            isnan(img1[index+1]) || isnan(img2[index+1]) ||
            isnan(img1[index+2]) || isnan(img2[index+2]))
         {
            std::cout << "# DEBUG > Detecting a NaN! Skipping pixel ("
                      << i << ", " << j << ")" << std::endl;
            continue;
         }

			float pix_error = 0.0;
         pix_error += pow(img2[(i + W*j)*3+0] - img1[(i + W*j)*3+0], 2);
			pix_error += pow(img2[(i + W*j)*3+1] - img1[(i + W*j)*3+1], 2);
			pix_error += pow(img2[(i + W*j)*3+2] - img1[(i + W*j)*3+2], 2);
         pix_error /= 3;

#ifdef DEBUG_SNR
			temp[i+j*W] = pix_error ;
#endif
			error_max  =  fmax(pix_error, error_max) ;
			error_mean += pix_error;
		}

   error_mean /= double(W * H) ;

#ifdef DEBUG_SNR
	t_EXR_IO<float>::SaveEXR("temp.exr", W, H, temp, 1) ;
	delete[] temp ;
#endif

   delete[] img1;
   delete[] img2 ;

	return error_mean ;
} ;
