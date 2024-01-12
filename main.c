#include<stdio.h>
#include<math.h>
#include"ppgdata.h"
#include"data_processing.h"

#define DATA_SIZE 1500
#define SAMPLE_RATE 25
#define WINDOW_SIZE 125 //5 seconds
#define ACCEL_THRESHOLD 0.39

int main() {
	//First, normalize the 1500 samples of the AFEData1
	double input_data[DATA_SIZE];
	double normalized_PPG[DATA_SIZE]; 
	
	for (int i = 0; i < DATA_SIZE; i++)input_data[i] = (double)AFEData1[i];
	calculateZScores(input_data, DATA_SIZE, normalized_PPG);
	
	//Then, using a customized FIR bandpass filter (0.4-4 Hz) on the normalized PPG data 
	double filtered_PPG[DATA_SIZE];
	firFilter(normalized_PPG, filtered_PPG, DATA_SIZE, fir_coefficients);	
	
	//Then, use the Accelerometer data to find the batches that are not noisy
	//Iterate through all the 1500 samples with a sliding window with size of 125 (1500/125=12 batches) samples
	double rr_diffs_sum = 0;
	int rr_count = 0;
	for (int j = 0; j < (DATA_SIZE / WINDOW_SIZE); j++)
	{
		double SquareRoot_Result[WINDOW_SIZE];

		for (int i = 0; i < WINDOW_SIZE; i++)
		{
			int current_index = j * WINDOW_SIZE + i;
			//Get the result of the Accelerometer data
			double x = (double)Accelerometer_X[current_index];
			double y = (double)Accelerometer_Y[current_index];
			double z = (double)Accelerometer_Z[current_index];

			SquareRoot_Result[i] = sqrt(x * x + y * y + z * z);
		}

		//Calculate the mean and standard deviation of Acceleometer data
		double zScores[WINDOW_SIZE];
		calculateZScores(SquareRoot_Result, WINDOW_SIZE, zScores);
		
		//Get the mean value of the Accel data within this batch
		double mean_abs = 0.0;
		for (int i = 0; i < WINDOW_SIZE; i++) {
			float abs_zscore = fabs(zScores[i]);
			mean_abs += abs_zscore;
		}
		mean_abs /= WINDOW_SIZE;

		//If the Accel mean of this batch is less than the threshold
		if (mean_abs < ACCEL_THRESHOLD) { //Consider the current batch is not noisy
			
			double ppg_batch[WINDOW_SIZE];

			for (int i = 0; i < WINDOW_SIZE; i++) {
				int current_index = j * WINDOW_SIZE + i;
				ppg_batch[i] = filtered_PPG[current_index];
			}
			calculateZScores(ppg_batch, WINDOW_SIZE, zScores);
			
			//Smooth the PPG data so that only one peak be catched during one pulse
			double smoothed_data[WINDOW_SIZE];
			smooth_data(zScores, WINDOW_SIZE, smoothed_data);

			//Calculate the peaks in this batch, and then get the heart rate
			int peaks[WINDOW_SIZE] = {0};
			int peak_count = find_peaks(smoothed_data, WINDOW_SIZE, peaks);

			rr_count += peak_count - 1; //In each batch, the number of RR is 1 less than the peaks
			for (int i = 0; i < peak_count - 1; i++) {
				rr_diffs_sum += (peaks[i + 1] - peaks[i]) / (double)SAMPLE_RATE;
			}
		}
		
	}
	double heart_rate = 60 / (rr_diffs_sum / (rr_count));

	printf("rr_count = %d, rr_diffs_sum = %lf, heart_rate = %f, hr_real = %f\n", rr_count, rr_diffs_sum, heart_rate, Real_HR);

	return 0;
}
