#include<stdio.h>
#include<math.h>
#include"ppgdata.h"
#include"data_processing.h"

#define DATA_SIZE 1250
#define PRE_DISCARDED_DATA_SIZE 250

#define SAMPLE_RATE 25
#define WINDOW_SIZE 125 //5 seconds
#define ACCEL_THRESHOLD 0.39

int main() {
	//First, normalize the 1250 samples of the AFEData1
	double input_raw_ppg[DATA_SIZE];
	double normalized_ppg[DATA_SIZE]; 
	
	for (int i = 0; i < DATA_SIZE; i++)input_raw_ppg[i] = (double)AFEData1[i + PRE_DISCARDED_DATA_SIZE];
	calculateZScores(input_raw_ppg, DATA_SIZE, normalized_ppg);
	
	//Then, using a customized FIR bandpass filter (0.4-4 Hz) on the normalized PPG data 
	double filtered_ppg[DATA_SIZE];
	firFilter(normalized_ppg, filtered_ppg, DATA_SIZE, fir_coefficients);
	
	//Then, do the normalization & filtering on the Accelerometer data
	double SquareRoot_Accel[DATA_SIZE];
	for (int i = 0; i < DATA_SIZE; i++)
	{
		//Get the result of the Accelerometer data
		double x = (double)Accelerometer_X[i + PRE_DISCARDED_DATA_SIZE];
		double y = (double)Accelerometer_Y[i + PRE_DISCARDED_DATA_SIZE];
		double z = (double)Accelerometer_Z[i + PRE_DISCARDED_DATA_SIZE];

		SquareRoot_Accel[i] = sqrt(x * x + y * y + z * z);
	}

	double normalized_Accel[DATA_SIZE];
	calculateZScores(SquareRoot_Accel, DATA_SIZE, normalized_Accel);
	
	double filtered_Accel[DATA_SIZE];
	firFilter(normalized_Accel, filtered_Accel, DATA_SIZE, fir_coefficients);
	
	//Then, use the Accelerometer data to find the batches that are not noisy
	//Iterate through all the 1250 samples with a sliding window with size of 125 (1250/125=10 batches) samples
	double rr_diffs_sum = 0;
	int rr_count = 0;

	for (int j = 0; j < (DATA_SIZE / WINDOW_SIZE); j++) //Iterate all 10 batches
	{
		double current_batch_Accel[WINDOW_SIZE];
		
		//Extract Accel data of current batch from the filtered_Accel[]
		for (int i = 0; i < WINDOW_SIZE; i++)
		{
			int current_index = j * WINDOW_SIZE + i;
			current_batch_Accel[i] = filtered_Accel[current_index];
		}
		
		//Get the mean value of the Accel data within this batch
		double mean_abs_Accel = 0.0;
		for (int i = 0; i < WINDOW_SIZE; i++) {
			float abs_Accel = fabs(current_batch_Accel[i]);
			mean_abs_Accel += abs_Accel;
		}
		mean_abs_Accel /= WINDOW_SIZE;

		//If the Accel mean of this batch is less than the threshold 0.39
		if (mean_abs_Accel < ACCEL_THRESHOLD) { //Consider the current batch is not noisy
			
			double current_batch_ppg[WINDOW_SIZE];

			for (int i = 0; i < WINDOW_SIZE; i++) {
				int current_index = j * WINDOW_SIZE + i;
				current_batch_ppg[i] = filtered_ppg[current_index];
			}
			
			//Smooth the PPG data so that only one peak be catched during each pulse
			double smoothed_ppg[WINDOW_SIZE];
			smooth_data(current_batch_ppg, WINDOW_SIZE, smoothed_ppg);

			//Calculate the peaks in this batch, and then get the heart rate
			int peaks[WINDOW_SIZE] = {0};
			int peak_count = find_peaks(smoothed_ppg, WINDOW_SIZE, peaks);

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
