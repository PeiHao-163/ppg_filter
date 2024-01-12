#pragma once
#ifndef _DATA_PROCESSING_H_
#define _DATA_PROCESSING_H_

#define FILTER_LENGTH 117  // Example length, adjust as needed

double calculateMean(double array[], int length);
double calculateStdDev(double array[], int length, double mean);
void calculateZScores(double array[], int length, double zScores[]);
int find_peaks(double data[], int size, int peaks[]);
void smooth_data(double data[], int size, double smoothed_data[]);
void firFilter(int* input, double* output, int input_length, double* coefficients);

#endif