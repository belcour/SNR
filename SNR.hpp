// Include STL
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <exception>
using namespace std;

// Include TinyEXR
#define TINYEXR_IMPLEMENTATION
#include <tinyexr/tinyexr.h>

// Include BOOST
#include <boost/log/trivial.hpp>

/* Deprecated: Old version of loading EXR images.
 */
float* loadImage(const std::string& name, int& W, int &H) {
   float* img = nullptr;
   const char* err;
   int ret = LoadEXR(&img, &W, &H, name.c_str(), &err);
   return img;
}

/* Exception type when loading EXRImages.
 */
struct ExceptionEXR : public std::exception {
	ExceptionEXR(const std::string& filename, const std::string& error) {
		msg = std::string("Error with file \'") + filename +
				std::string("\': ") +	error;
	}

	const char* what() const noexcept {
		return msg.c_str();
	}

	std::string msg;
};

/* Loading an EXR image using tinyexr. The image needs to be located at
 * filename `name`.
 *
 * Return the EXRImage if the image can be loaded. If the image cannot
 * be loaded, an ExceptionEXR is thrown.
 */
EXRImage LoadImage(const std::string& name) {
	const char* input = name.c_str();
	const char* err;

	EXRImage image;
	InitEXRImage(&image);

	int ret = ParseMultiChannelEXRHeaderFromFile(&image, input, &err);
	if (ret != 0) {
		std::string err_str(err);
		throw ExceptionEXR(name, err_str);
	}

	for (int i = 0; i < image.num_channels; i++) {
		image.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
	}

	ret = LoadMultiChannelEXRFromFile(&image, input, &err);
	if (ret != 0) {
		std::string err_str(err);
		throw ExceptionEXR(name, err_str);
	}

	return image;
}

int b = 0;
void setBorder(int border)
{
	b = border ;
}

/* Requires a MetricClass object. This object must have two methods:
 *     void operator(vector pixelA, vector pixelB)
 *     dict Statistics() const
 *
 * The operator accumulate statistics between images.
 * The statistics return a dictionnary of couple type and value.
 */
template<class MetricClass>
float Metric(const std::string& query, const std::string& ref) {

   // Open files
   auto qryImg = LoadImage(query);
   auto refImg = LoadImage(ref);

   // Check if the images are coherent
   // Width, Height, and Number of channels must match
   if(qryImg.width        != refImg.width        ||
      qryImg.height       != refImg.height       ||
      qryImg.num_channels != refImg.num_channels) {
      throw ExceptionEXR(ref + std::string(" and ") + ref,
                         std::string("Files do not match"));
   }

   // Create the metric object
   MetricClass metric(qryImg.num_channels);
   std::vector<float> pixelQry, pixelRef;
   pixelQry.reserve(qryImg.num_channels);
   pixelRef.reserve(qryImg.num_channels);

   // Loop over the pixels and perform a metric between the query
   // and the reference pixel.
   const int N = qryImg.width * qryImg.height;
   const int channel = 0;
   for(int i=0; i<N; ++i) {
      for(int k=0; k<qryImg.num_channels; ++k) {
         pixelQry[k] = (float)qryImg.images[k][i];
         pixelRef[k] = (float)refImg.images[k][i];
      }

      metric(pixelQry, pixelRef);
   }

   return metric.Statistics();
}

/* Statistics class to compute the average SNR.
 */
struct SnrStatistics {
   float signal, error;
   unsigned int nb;

   SnrStatistics(int numChannels) {
      signal = 0.0f;
      error  = 0.0f;
      nb     = 0;
   }

   void operator()(const std::vector<float>& qry,
                   const std::vector<float>& ref) {
      if(isnan(ref[0]) || isnan(qry[0]) ||
         isnan(ref[1]) || isnan(qry[1]) ||
         isnan(ref[2]) || isnan(qry[2])) {
         BOOST_LOG_TRIVIAL(warning) << "Detecting a NaN! Skipping pixel";
         return;
      }

      float avg_error = (pow(ref[0] - qry[0], 2)
                       + pow(ref[1] - qry[1], 2)
                       + pow(ref[2] - qry[2], 2)) / 3.0f;

      float avg_sign  = (pow(ref[0], 2)
                       + pow(ref[1], 2)
                       + pow(ref[2], 2)) / 3.0f;

      error  = (error*nb  + avg_error) / float(nb+1);
      signal = (signal*nb + avg_sign)  / float(nb+1);
      ++nb;
   }

   float Statistics() const {
      return signal / error;
   }
};



float SNR(const string& image, const string& reference)
{
	int W, H ;
	float *img1, *img2 ;

   img1 = loadImage(reference, W, H);
	if(img1 == nullptr) {
		BOOST_LOG_TRIVIAL(error) << "Unable to load the reference image";
		return 0 ;
	}

	int Wtemp, Htemp ;
   img2 = loadImage(image, Wtemp, Htemp);
	if(img2 == nullptr || W != Wtemp || H != Htemp) {
		BOOST_LOG_TRIVIAL(error) << "Unable to load the query image";
		return 0 ;
	}

	float error  = 0.0f ;
	float signal = 0.0f ;
	float smax   = 0.0f ;
	for(int i=b; i<W-b; ++i)
		for(int j=b; j<H-b; ++j) {

         const int index = (i + W*j)*4;
         if(isnan(img1[index+0]) || isnan(img2[index+0]) ||
            isnan(img1[index+1]) || isnan(img2[index+1]) ||
            isnan(img1[index+2]) || isnan(img2[index+2])) {
            BOOST_LOG_TRIVIAL(warning) << "# DEBUG > Detecting a NaN! Skipping pixel ("
                                       << i << ", " << j << ")";
            continue;
         }

			error  += pow(img2[index+0] - img1[index+0], 2) ;
			error  += pow(img2[index+1] - img1[index+1], 2) ;
			error  += pow(img2[index+2] - img1[index+2], 2) ;
			signal += pow(img1[index+0], 2) ;
			signal += pow(img1[index+1], 2) ;
			signal += pow(img1[index+2], 2) ;
			smax = fmax(img1[index+0], smax) ;
			smax = fmax(img1[index+1], smax) ;
			smax = fmax(img1[index+2], smax) ;
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
	if(img1 == nullptr) {
		BOOST_LOG_TRIVIAL(error) << "Unable to load the reference image";
		return 0 ;
	}

	int Wtemp, Htemp ;
   img2 = loadImage(image, Wtemp, Htemp);
	if(img2 == nullptr || W != Wtemp || H != Htemp) {
		BOOST_LOG_TRIVIAL(error) << "Unable to load the query image";
		return 0 ;
	}

	double error_max  = 0.0 ;
	double error_mean = 0.0 ;
	for(int i=0; i<W; ++i)
		for(int j=0; j<H; ++j) {

         const int index = (i + W*j)*4;
         if(isnan(img1[index+0]) || isnan(img2[index+0]) ||
            isnan(img1[index+1]) || isnan(img2[index+1]) ||
            isnan(img1[index+2]) || isnan(img2[index+2])) {
            BOOST_LOG_TRIVIAL(warning) << "# DEBUG > Detecting a NaN! Skipping pixel ("
                                       << i << ", " << j << ")";
            continue;
         }

			float pix_error = 0.0;
         pix_error += pow(img2[index+0] - img1[index+0], 2);
			pix_error += pow(img2[index+1] - img1[index+1], 2);
			pix_error += pow(img2[index+2] - img1[index+2], 2);
         pix_error /= 3;

			error_max  =  fmax(pix_error, error_max) ;
			error_mean += pix_error;
		}

   error_mean /= double(W * H) ;

   delete[] img1;
   delete[] img2 ;

	return error_mean ;
} ;
