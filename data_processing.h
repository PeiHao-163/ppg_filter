#pragma once
#ifndef _DATA_PROCESSING_H_
#define _DATA_PROCESSING_H_

#define FILTER_LENGTH 117  // Example length, adjust as needed

float calculateMean(float array[], int length);
float calculateStdDev(float array[], int length, float mean);
void calculateZScores(float array[], int length, float zScores[]);
int find_peaks(float data[], int size, int peaks[]);
void smooth_data(float data[], int size, float smoothed_data[]);
void firFilter(float* input, float* output, int input_length, float* coefficients);

#endif