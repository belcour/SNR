// Include STL
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <exception>
using namespace std;

// Include TinyEXR
#define TINYEXR_IMPLEMENTATION
#include <tinyexr/tinyexr.h>

// Include BOOST
#include <boost/log/trivial.hpp>

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

   for(int k=0; k<qryImg.num_channels; ++k) {
      if(strcmp(qryImg.channel_names[k], refImg.channel_names[k]) != 0) {
         throw ExceptionEXR(ref + std::string(" and ") + ref,
                            std::string("Have different color channels"));
      }
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
         // Convert unsigned int pointers to float pointers before
         // accessing the values.
         float* qry = (float*)qryImg.images[k];
         float* ref = (float*)refImg.images[k];
         pixelQry[k] = qry[i];
         pixelRef[k] = ref[i];
      }

      metric(pixelQry, pixelRef);
   }

   return metric.Statistics();
}

/* Statistics class to compute the average SNR in decibels.
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
      return 10.0f * log(signal / error);
   }
};

/* Statistics class to compute the average RMSE per channel.
 */
struct RmseStatistics {
   float sqr_error;
   unsigned int nb;

   RmseStatistics(int numChannels) {
      sqr_error = 0.0f;
      nb        = 0;
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

      sqr_error  = (sqr_error*nb  + avg_error) / float(nb+1);
      ++nb;
   }

   float Statistics() const {
      return sqrt(sqr_error);
   }
};
