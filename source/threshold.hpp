#pragma once

#include <opencv2/core.hpp>

class Threshold
{
public:
	/**
	 * Otsu treshold algorithm with mask support.
	 */
	static double otsuMask(const cv::Mat1b src, const cv::Mat1b& mask)
	{
		//Colors
		const int N = 256;

		int M = 0;

		//Create the image histogram
		int h[N] = {0};
		for(int i = 0; i < src.rows; i++)
		{
			const uchar* psrc = src.ptr(i);
			const uchar* pmask = mask.ptr(i);
			for(int j = 0; j < src.cols; j++)
			{
				if(pmask[j])
				{
					h[psrc[j]]++;
					++M;
				}
			}
		}

		double mu = 0, scale = 1.0 / (M);
		for(int i = 0; i < N; i++)
		{
			mu += i * (double)h[i];
		}

		mu *= scale;
		double mu1 = 0, q1 = 0;
		double max_sigma = 0, max_val = 0;

		for(int i = 0; i < N; i++)
		{
			double p_i, q2, mu2, sigma;

			p_i = h[i] * scale;
			mu1 *= q1;
			q1 += p_i;
			q2 = 1.0 - q1;

			if(cv::min(q1, q2) < FLT_EPSILON || cv::max(q1, q2) > 1.0 - FLT_EPSILON)
			{
				continue;
			}

			mu1 = (mu1 + i * p_i) / q1;
			mu2 = (mu - q1 * mu1) / q2;
			sigma = q1 * q2 * (mu1 - mu2) * (mu1 - mu2);

			if(sigma > max_sigma)
			{
				max_sigma = sigma;
				max_val = i;
			}
		}

		return max_val;
	}

	/**
	 * Corsk specific automatic treshold calculation.
	 *
	 * Differently from the OTSU algorithm that always aceppts a separations between the values this algorithm can allow situations were there is no sparation.
	 *
	 * @param src Input image.
	 * @param mask Mask image.
	 * @param min_color_separation Minimum separation between the colors found to get a treshold, if the diff exceds this limit 0 is returned.
	 * @param neighborhood Neighborhood to analyse when creating the comulative histogram.
	 * @param min_neighborhood_separation Min separation between the neighbors.
	 * @param balance Ratio between the two colors with more occurences.
	 * @param min_occorrences_ratio Ratio of occurences between the two colors with more occurences.
	 */
	static double histogram(const cv::Mat1b src, const cv::Mat1b& mask, int min_color_separation = 15, int neighborhood = 15, int min_neighborhood_separation = 15, double balance = 0.5, double min_occorrences_ratio = 0.65)
	{
		const int N = 256;
		int histogram[N] = {0};
		
		//Create the image histogram
		for(int i = 0; i < src.rows; i++)
		{
			const uchar* psrc = src.ptr(i);
			const uchar* pmask = mask.ptr(i);
			for(int j = 0; j < src.cols; j++)
			{
				if(pmask[j])
				{
					histogram[psrc[j]]++;
				}
			}
		}

		//Count in the neighborhood
		int count[N] = {0};
		int colors[N] = {0};
		
		//Neighborhood to be analysed
		int neighborhood_half = neighborhood / 2;
		int start = neighborhood_half;
		int end = N - neighborhood_half;

		for(int i = start; i < end; i++)
		{
			int sum_count = 0;
			int sum_color = 0;
			int c = 0;

			//Analyse the neighborhood
			for(int j = i - neighborhood_half; j < i + neighborhood_half; j++)
			{
				sum_color += i;
				sum_count += histogram[i];
				c++;
			}

			colors[i] = sum_color / c;
			count[i] = sum_count;
		}

		//Sort the array from bigger to smaller
		for(int i = 0; i < N; i++)
		{
			for(int j = i; j < N; j++)
			{
				if(count[i] < count[j])
			 	{
			 		int temp = count[i];
			 		count[i] = count[j];
			 		count[j] = temp;

			 		temp = colors[i];
			 		colors[i] = colors[j];
			 		colors[j] = temp;
			 	}
			}
		}

		//Colors with more occurences
		int high = colors[0];
		int low = -1, low_count = 0;
		
		//Select the low color (the next color with more occurences)
		for(int i = 1; i < N; i++)
		{
			int highc = cv::max(colors[i], high);
			int lowc = cv::min(colors[i], high);
			
			if((highc - lowc) < min_neighborhood_separation)
			{
				continue;
			}
			
			low_count = count[i];
			low = colors[i];
			break;
		}

		double diff = (high > low) ? high - low : low - high;
		double ratio = (double)low_count / (double)count[0];
		
		if(diff < min_color_separation || ratio < min_occorrences_ratio || low == -1)
		{
			//cout << "Ratio:" << ratio << endl;
			//cout << "Diff:" << diff << ", MinDiff:" << min_color_separation << ", High:" << high << ", Low:" << low << endl;
			return 0;
		}
		else
		{
			//cout << "High:" << high << "(" << count[0] << "), Low:" << low << "(" << low_count << ")" << endl;
		}

		return low + (diff * balance);
	}
};