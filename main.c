#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "dirent.h"
#include "ppgdata.h"
#include "data_processing.h"

#define DATA_SIZE 1250
#define PRE_DISCARDED_DATA_SIZE 250
#define MAX_LINE_LENGTH 1000        
#define NUM_ROWS 1500 

#define SAMPLE_RATE 25
#define WINDOW_SIZE 125 //5 seconds
#define ACCEL_THRESHOLD 0.39

typedef struct {
	float hr_measured;
	float discard_ratio;
} HRValues;

void hr_estimation(int* ppg_data, int* accel_x, int* accel_y, int* accel_z, HRValues* pHRValues)
{
	//First, normalize the 1250 samples of the AFEData1
	float input_raw_ppg[DATA_SIZE];
	float normalized_ppg[DATA_SIZE];

	for (int i = 0; i < DATA_SIZE; i++)input_raw_ppg[i] = (float)ppg_data[i + PRE_DISCARDED_DATA_SIZE];
	calculateZScores(input_raw_ppg, DATA_SIZE, normalized_ppg);

	//Then, using a customized FIR bandpass filter (0.4-4 Hz) on the normalized PPG data 
	float filtered_ppg[DATA_SIZE];
	firFilter(normalized_ppg, filtered_ppg, DATA_SIZE, fir_coefficients);

	//Then, do the normalization & filtering on the Accelerometer data
	float SquareRoot_Accel[DATA_SIZE];
	for (int i = 0; i < DATA_SIZE; i++)
	{
		//Get the result of the Accelerometer data
		float x = (float)accel_x[i + PRE_DISCARDED_DATA_SIZE];
		float y = (float)accel_y[i + PRE_DISCARDED_DATA_SIZE];
		float z = (float)accel_z[i + PRE_DISCARDED_DATA_SIZE];

		SquareRoot_Accel[i] = sqrt(x * x + y * y + z * z);
	}

	float normalized_Accel[DATA_SIZE];
	calculateZScores(SquareRoot_Accel, DATA_SIZE, normalized_Accel);

	float filtered_Accel[DATA_SIZE];
	firFilter(normalized_Accel, filtered_Accel, DATA_SIZE, fir_coefficients);

	//Then, use the Accelerometer data to find the batches that are not noisy
	//Iterate through all the 1250 samples with a sliding window with size of 125 (1250/125=10 batches) samples
	float rr_diffs_sum = 0;
	int rr_count = 0;
	int usable_batch_count = 0;

	for (int j = 0; j < (DATA_SIZE / WINDOW_SIZE); j++) //Iterate all 10 batches
	{
		float current_batch_Accel[WINDOW_SIZE];

		//Extract Accel data of current batch from the filtered_Accel[]
		for (int i = 0; i < WINDOW_SIZE; i++)
		{
			int current_index = j * WINDOW_SIZE + i;
			current_batch_Accel[i] = filtered_Accel[current_index];
		}

		//Get the mean value of the Accel data within this batch
		float mean_abs_Accel = 0.0;
		for (int i = 0; i < WINDOW_SIZE; i++) {
			float abs_Accel = fabs(current_batch_Accel[i]);
			mean_abs_Accel += abs_Accel;
		}
		mean_abs_Accel /= WINDOW_SIZE;

		//If the Accel mean of this batch is less than the threshold 0.39
		if (mean_abs_Accel < ACCEL_THRESHOLD) { //Consider the current batch is not noisy
			usable_batch_count++; //Count the number of usable batches
			float current_batch_ppg[WINDOW_SIZE];

			for (int i = 0; i < WINDOW_SIZE; i++) {
				int current_index = j * WINDOW_SIZE + i;
				current_batch_ppg[i] = filtered_ppg[current_index];
			}

			//Smooth the PPG data so that only one peak be catched during each pulse
			float smoothed_ppg[WINDOW_SIZE];
			smooth_data(current_batch_ppg, WINDOW_SIZE, smoothed_ppg);

			//Calculate the peaks in this batch, and then get the heart rate
			int peaks[WINDOW_SIZE] = { 0 };
			int peak_count = find_peaks(smoothed_ppg, WINDOW_SIZE, peaks);

			rr_count += peak_count - 1; //In each batch, the number of RR is 1 less than the peaks
			for (int i = 0; i < peak_count - 1; i++) {
				rr_diffs_sum += (peaks[i + 1] - peaks[i]) / (float)SAMPLE_RATE;
			}
		}
	}
	float heart_rate = 60 / (rr_diffs_sum / (rr_count));

	float discard_ratio = 1 - usable_batch_count / (float)(DATA_SIZE / WINDOW_SIZE);

	pHRValues->hr_measured = heart_rate;
	pHRValues->discard_ratio = discard_ratio;
}

int process_csv_file(const char* file_path, int* data_afe1, int* data_accel_x, int* data_accel_y, int* data_accel_z) {
	FILE* file = fopen(file_path, "r");
	if (file == NULL) {
		perror("Error opening file");
		return EXIT_FAILURE;
	}
	
	int real_hr = 0;	
	int rowIndex = 0;
	int isFirstLine = 1;
	char line[MAX_LINE_LENGTH];

	while (fgets(line, MAX_LINE_LENGTH, file)) {
		if (isFirstLine) {
			// Skip the first line (column names)
			isFirstLine = 0;
			continue;
		}
		
		char* context = NULL;
		char* token = strtok_s(line, ",", &context);
		int colIndex = 0;
		
		while (token != NULL && rowIndex < NUM_ROWS) {
			if (colIndex == 0)data_accel_x[rowIndex] = atoi(token);
			else if (colIndex == 1)data_accel_y[rowIndex] = atoi(token);
			else if (colIndex == 2)data_accel_z[rowIndex] = atoi(token);
			else if (colIndex == 3)data_afe1[rowIndex] = atoi(token);
			else if (colIndex == 7 && rowIndex == 0)real_hr = atoi(token);

			token = strtok_s(NULL, ",", &context);			
			colIndex++;
		}
		rowIndex++;
	}

	fclose(file);
	return real_hr;
}

int main() {
	DIR* dir;
	struct dirent* ent;
	char folder_path[] = "C:/Users/peiha/Desktop/C_Project/ppg_data";
	char file_path[1024];

	//Data for saving
	int real_hr_csv[1000] = { 0 };
	float hr_measured_csv[1000] = { 0 };
	float discard_ratio_csv[1000] = { 0 };
	
	// Open the file for writing
	FILE* file_csv = fopen("hr_result.csv", "w"); 

	if (file_csv == NULL) {
		printf("Error opening csv file!\n");
		return 1;
	}

	if ((dir = opendir(folder_path)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (strstr(ent->d_name, ".csv") != NULL) {
				snprintf(file_path, sizeof(file_path), "%s/%s", folder_path, ent->d_name);
				//process_csv_file(file_path);
				
				//if (strcmp(file_path, "C:/Users/peiha/Desktop/C_Project/ppg_data/1630905988128_A.csv") == 0) {
					int real_hr = 0;
					int data_afe1[NUM_ROWS] = {0};
					int data_accel_x[NUM_ROWS] = { 0 };
					int data_accel_y[NUM_ROWS] = { 0 };
					int data_accel_z[NUM_ROWS] = { 0 };
					
					real_hr = process_csv_file(file_path, data_afe1, data_accel_x, data_accel_y, data_accel_z);
					
					HRValues return_values = { 0, 0 };
					hr_estimation(data_afe1, data_accel_x, data_accel_y, data_accel_z, &return_values);
					printf("hr_real = %d, hr_measured = %f, discard_ratio = %f\n", real_hr, return_values.hr_measured, return_values.discard_ratio);
					fprintf(file_csv, "%d,%f,%f\n", real_hr, return_values.hr_measured, return_values.discard_ratio);
				//}
			}
		}
		closedir(dir);
	}
	else {
		perror("Unable to open directory");
		return EXIT_FAILURE;
	}

	fclose(file_csv);

	return EXIT_SUCCESS;

	//float real_hr = 0;
	//HRValues return_values = {0, 0};
	//
	//hr_estimation(AFEData1, Accelerometer_X, Accelerometer_Y, Accelerometer_Z, &return_values);
	//printf("hr_real = %f, hr_measured = %f, discard_ratio = %f\n", Real_HR, return_values.hr_measured, return_values.discard_ratio);
	//return 0;
}
