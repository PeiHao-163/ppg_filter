#include <math.h>
#include"data_processing.h"

// Function to calculate the mean of an array
double calculateMean(double array[], int length) {
	double sum = 0.0;
	for (int i = 0; i < length; i++) {
		sum += array[i];
	}
	return sum / length;
}

// Function to calculate the standard deviation of an array
double calculateStdDev(double array[], int length, double mean) {
	double sum_squared_diff = 0.0;
	for (int i = 0; i < length; i++) {
		double diff = array[i] - mean;
		sum_squared_diff += diff * diff;
	}
	return sqrt(sum_squared_diff / length);
}

// Function to calculate the z-scores for an array
void calculateZScores(double array[], int length, double zScores[]) {
	double mean = calculateMean(array, length);
	double stdDev = calculateStdDev(array, length, mean);

	for (int i = 0; i < length; i++) {
		zScores[i] = (array[i] - mean) / stdDev;
	}
}

// Function to find peaks in the PPG data
int find_peaks(double data[], int size, int peaks[]) {
	int peak_count = 0;
	for (int i = 1; i < size - 1; i++) {
		if (data[i] > data[i - 1] && data[i] > data[i + 1]) { // A simple peak detection
			peaks[peak_count++] = i;
		}
	}
	return peak_count;
}

// Function to calculate heart rate
//double calculate_heart_rate(int peaks[], int peak_count) {
//	double total_time = 0;
//	for (int i = 0; i < peak_count - 1; i++) {
//		total_time += (peaks[i + 1] - peaks[i]) / (double)SAMPLE_RATE;
//	}
//	double average_time_between_peaks = total_time / (peak_count - 1);
//	return 60 / average_time_between_peaks;
//}

// Function to smooth the data
void smooth_data(double data[], int size, double smoothed_data[]) {
	int window_size = 5; // Example window size for smoothing
	for (int i = 0; i < size; i++) {
		double sum = 0;
		int count = 0;
		for (int j = -window_size; j <= window_size; j++) {
			int index = i + j;
			if (index >= 0 && index < size) {
				sum += data[index];
				count++;
			}
		}
		smoothed_data[i] = sum / count;
	}
}

//void firFilter(int* input, double* output, int length, double* coefficients) {
//	double sum;
//	for (int i = 0; i < length; i++) {
//		sum = 0;
//		for (int j = 0; j < FILTER_LENGTH; j++) {
//			if (i - j >= 0) {
//				sum += input[i - j] * coefficients[j];
//			}
//		}
//		output[i] = sum;
//	}
//}