#ifndef EXAMPLEOFCONFIGFILE_H_
#define EXAMPLEOFCONFIGFILE_H_

#include "ConfigFileWriter.h"



// Example of how to create a main config file
void CreateNewMainConfig()
{
	CConfigFileWriter * Config = new CConfigFileWriter();
	
	Config->CreateNewSection("CommonParameters");
	Config->SetName("CommonParameters", "Description", "MainConfig"); // Description of the file (MainConfig)

	Config->CreateNewSection("RawDataChannels");
	Config->SetName("RawDataChannels", "BinDataFolder", "/Data/TmpData/"); // Path to the folder, where bin data files to process are placed
	Config->SetName("RawDataChannels", "OutputFolder", "/Data/Output"); // Path to the folder for output files
	Config->SetName("RawDataChannels", "Postfix", "_[Recognition]"); // Postfix added to names of output files
	Config->SetLong("RawDataChannels", "OutFileFormat", 0); // 0: old library format, 1: new format (with extra symbols)
	Config->SetLong("RawDataChannels", "GUIMode", 4);  // 0 - no output GUI file, 1 - detection stage, 2 - recognition stage
	Config->SetLong("RawDataChannels", "bool_ApplyMapFiltering", 0);  // 1 - apply spatio-temporal filtering of events (equivalent to UseMapFile)
	Config->SetLong("RawDataChannels", "bool_NeedPrintInfo", 1); // 1 - print out aux information during the processing to the output text file and console (not recommended in time benchmarks)
	Config->SetLong("RawDataChannels", "bool_GetAllDataCells", 0); // 0 - print out all data within the given range in the MathCad format (transposed); 1 - print out only detected events in the Library format
	Config->SetFloat("RawDataChannels", "GuiSNRMultiplier", 100.0); // set SNR multiplier in the GUI text files. Note: saturation may occur if a scaled SNR value is more than 255 (max brightness)
	Config->SetLong("RawDataChannels", "StartFrame", 0); // default = 0; thus this line is not necessary
	Config->SetLong("RawDataChannels", "EndFrame", 800); // default = End of the file. If the input value exceeds the maximal frame, then EndFrame = MaxFrame
	Config->AppendTextToSection("RawDataChannels", ""); // Insert an empty line

	Config->CreateNewSection("DataFusion");
	Config->SetLong("DataFusion", "N_Cores", 6); // Define the number of threads to perform the task
	Config->SetLong("DataFusion", "N_Zones", 2); // Define the number of spatial zones having different properties
	Config->SetLong("DataFusion", "FrameSize", 512); // Frame size (512, 1024)
	Config->SetLong("DataFusion", "FusionSize", 4); // Number of raw channels to be fused 
	Config->SetLong("DataFusion", "bool_AreFramesOverlapped", 0); // 0 - frames are sequential; 1 - frames are half-overlapped 
	Config->SetFloat("DataFusion", "SaturationThreshold", 0.05); // Saturation threshold is required to discard saturated signals. The value is usually 5% - 10%. Note: this parameter may be assigned to a certain zone 
	Config->AppendTextToSection("DataFusion", ""); // Insert an empty line
	Config->AppendTextToSection("DataFusion", "//___________________ Zone 0 ______________________"); // Print a comment denoting description of a spatial zone
	Config->SetLong("DataFusion", "StartRawChannel", 0, 0); // Set the start channel for the zone
	Config->SetLong("DataFusion", "EndRawChannel", 0, 299); // Set the end channel for the zone
	Config->AppendTextToSection("DataFusion", ""); // Insert an empty line
	Config->AppendTextToSection("DataFusion", "//___________________ Zone 1 ______________________"); // Print a comment denoting description of a spatial zone
	Config->SetLong("DataFusion", "StartRawChannel", 1, 300); // Set the start channel for the zone
	Config->SetLong("DataFusion", "EndRawChannel", 1, 749); // Set the end channel for the zone
	Config->AppendTextToSection("DataFusion", ""); // Insert an empty line
	
	Config->CreateNewSection("Detection");
	Config->SetLong("Detection", "N_Cores", 6); // Define the number of threads to perform the task
	Config->SetLong("Detection", "N_Zones", 3); // Define the number of spatial zones having different properties
	Config->SetLong("Detection", "bool_EnableTransientDetection", 1); // 0 - disable transient detection algorithm; 1 - enable transient detection algorithm 
	// Config->SetName("Detection", "EqualizerFileName", "/Data/AuxFiles/PSD_Equalizer.txt"); // Set path to the text file containing equalizer values. Note: this parameter may be assigned to a certain zone
	Config->SetFloat("Detection", "DetectionTimeRate", 0); // 0 - disabled. the parameter is used to control and constrain the detection rate in low sensitivity mode. Usually 0.01. Note: this parameter may be assigned to a certain zone    
	Config->SetLong("Detection", "SpatialGroupSize", 6); // Number of fused channels in one spatial group (used in interference suppression techiques)
	Config->AppendTextToSection("Detection", ""); // Insert an empty line
	// Note: three zones described below have identical parameters. In practice, their parameters may not be the same.
	// Common parameters may be listed outside of the zone description, without assignment to a particular zone. 
	Config->AppendTextToSection("Detection", "//___________________ Zone 0 ______________________"); // Print a comment denoting description of a spatial zone
	Config->SetLong("Detection", "StartFusedChannel", 0, 0); // Set the start channel for the zone
	Config->SetLong("Detection", "StartFusedChannel", 0, 49); // Set the end channel for the zone
	Config->SetLong("Detection", "NumberOfBins_Spectral", 0, 16); // Number of bins in the spectral detection algorithms (8 or 16)
	Config->SetLong("Detection", "NumberOfBins_Transient", 0, 16); // Number of bins in the transient detection algorithms (8 or 16)
	Config->SetLong("Detection", "TypeOfProcessing", 0, 0); // Type of interferense suppression: 0 - no, 1 - simple whitening, 2 - advanced whiterning, 3 - adaptive filtering
	Config->SetFloat("Detection", "Threshold_SpectralDetection", 0, 0.178); // Threshold for the spectral detection algorithm
	Config->SetFloat("Detection", "Threshold_TransientDetection", 0, 0.737); // Threshold for the transient detection algorithm
	Config->SetFloat("Detection", "InterferenceCorrTimeRate", 0, 0.03); // Usually 0.02-0.03 (0 - disabled). The higher the value, the more aggressive interference suppression
	Config->SetFloat("Detection", "FadingCorrTimeRate", 0, 0.02); // Usually 0.02-0.03 (0 - disabled). The higher the value, the more aggressive fading compensation
	Config->SetLong("Detection", "TypeOfWindowFunction", 0, 4);  // Window type: 0 - none (rectangular); 1 - Hann (q = 1); 2 - Hann (q = 0.5); 3 - Hann (q = 0.25), 4 - Hamming
	Config->AppendTextToSection("Detection", ""); // Insert an empty line
	Config->AppendTextToSection("Detection", "//___________________ Zone 1 ______________________"); // Print a comment denoting description of a spatial zone
	Config->SetLong("Detection", "StartFusedChannel", 1, 50); // Set the start channel for the zone
	Config->SetLong("Detection", "StartFusedChannel", 1, 99); // Set the end channel for the zone
	Config->SetLong("Detection", "NumberOfBins_Spectral", 1, 16); // Number of bins in the spectral detection algorithms (8 or 16)
	Config->SetLong("Detection", "NumberOfBins_Transient", 1, 16); // Number of bins in the transient detection algorithms (8 or 16)
	Config->SetLong("Detection", "TypeOfProcessing", 1, 0); // Type of interferense suppression: 0 - no, 1 - simple whitening, 2 - advanced whiterning, 3 - adaptive filtering
	Config->SetFloat("Detection", "Threshold_SpectralDetection", 1, 0.178); // Threshold for the spectral detection algorithm
	Config->SetFloat("Detection", "Threshold_TransientDetection", 1, 0.737); // Threshold for the transient detection algorithm
	Config->SetFloat("Detection", "InterferenceCorrTimeRate", 1, 0.03); // Usually 0.02-0.03 (0 - disabled). The higher the value, the more aggressive interference suppression
	Config->SetFloat("Detection", "FadingCorrTimeRate", 1, 0.02); // Usually 0.02-0.03 (0 - disabled). The higher the value, the more aggressive fading compensation
	Config->SetLong("Detection", "TypeOfWindowFunction", 1, 4);  // Window type: 0 - none (rectangular); 1 - Hann (q = 1); 2 - Hann (q = 0.5); 3 - Hann (q = 0.25), 4 - Hamming
	Config->AppendTextToSection("Detection", ""); // Insert an empty line
	Config->AppendTextToSection("Detection", "//___________________ Zone 2 ______________________"); // Print a comment denoting description of a spatial zone
	Config->SetLong("Detection", "StartFusedChannel", 2, 100); // Set the start channel for the zone
	Config->SetLong("Detection", "StartFusedChannel", 2, 186); // Set the end channel for the zone
	Config->SetLong("Detection", "NumberOfBins_Spectral", 2, 16); // Number of bins in the spectral detection algorithms (8 or 16)
	Config->SetLong("Detection", "NumberOfBins_Transient", 2, 16); // Number of bins in the transient detection algorithms (8 or 16)
	Config->SetLong("Detection", "TypeOfProcessing", 2, 0); // Type of interferense suppression: 0 - no, 1 - simple whitening, 2 - advanced whiterning, 3 - adaptive filtering
	Config->SetFloat("Detection", "Threshold_SpectralDetection", 2, 0.178); // Threshold for the spectral detection algorithm
	Config->SetFloat("Detection", "Threshold_TransientDetection", 2, 0.737); // Threshold for the transient detection algorithm
	Config->SetFloat("Detection", "InterferenceCorrTimeRate", 2, 0.03); // Usually 0.02-0.03 (0 - disabled). The higher the value, the more aggressive interference suppression
	Config->SetFloat("Detection", "FadingCorrTimeRate", 2, 0.02); // Usually 0.02-0.03 (0 - disabled). The higher the value, the more aggressive fading compensation
	Config->SetLong("Detection", "TypeOfWindowFunction", 2, 4);  // Window type: 0 - none (rectangular); 1 - Hann (q = 1); 2 - Hann (q = 0.5); 3 - Hann (q = 0.25), 4 - Hamming
	Config->AppendTextToSection("Detection", ""); // Insert an empty line

	Config->CreateNewSection("SetOfRecognizers");
	Config->SetLong("SetOfRecognizers", "N_Recognizers", 3); // Number of available recognizers 
	Config->SetName("SetOfRecognizers", "RecognitionInfoFileName", 0, "/Data/ConfigFiles/RecInfo1.txt"); // Recognizer #0
	Config->SetName("SetOfRecognizers", "RecognitionInfoFileName", 1, "/Data/ConfigFiles/RecInfo2.txt"); // Recognizer #1
	Config->SetName("SetOfRecognizers", "RecognitionInfoFileName", 2, "/Data/ConfigFiles/RecInfo3.txt"); // Recognizer #2
	Config->AppendTextToSection("SetOfRecognizers", ""); // Insert an empty line
	
	Config->CreateNewSection("SignalRecognition");
	Config->SetLong("SignalRecognition", "N_Cores", 6); // Define the number of threads to perform the task
	Config->SetLong("SignalRecognition", "N_Zones", 3); // Define the number of spatial zones assigned to defined recognizers
	Config->SetLong("SignalRecognition", "NMax_Classes", 6); // Maximal number of classes (without taking into account the unknown class). Note: number of classes in the defined recognizers is equal or less than this value.
	Config->SetLong("SignalRecognition", "DecisionType", 2); // Type of output decision. 0 - hard, 1 - soft, 2 - both
	Config->AppendTextToSection("SignalRecognition", ""); // Insert an empty line
	// Note: different zones may be assigned to different recognizers or to the same recognizer 
	Config->AppendTextToSection("SignalRecognition", "//___________________ Zone 0 ______________________"); // Print a comment denoting description of a spatial zone
	Config->SetLong("SignalRecognition", "StartFusedChannel", 0, 0); // Set the start channel for the zone
	Config->SetLong("SignalRecognition", "EndFusedChannel", 0, 70); // Set the end channel for the zone
	Config->SetLong("SignalRecognition", "RecognizerID", 0, 0); // Assign a recognizer (by its id in the list of recognizers) to this zone
	Config->AppendTextToSection("SignalRecognition", ""); // Insert an empty line
	Config->AppendTextToSection("SignalRecognition", "//___________________ Zone 1 ______________________"); // Print a comment denoting description of a spatial zone
	Config->SetLong("SignalRecognition", "StartFusedChannel", 1, 71); // Set the start channel for the zone
	Config->SetLong("SignalRecognition", "EndFusedChannel", 1, 120); // Set the end channel for the zone
	Config->SetLong("SignalRecognition", "RecognizerID", 1, 1); // Assign a recognizer (by its id in the list of recognizers) to this zone
	Config->AppendTextToSection("SignalRecognition", ""); // Insert an empty line
	Config->AppendTextToSection("SignalRecognition", "//___________________ Zone 2 ______________________"); // Print a comment denoting description of a spatial zone
	Config->SetLong("SignalRecognition", "StartFusedChannel", 2, 121); // Set the start channel for the zone
	Config->SetLong("SignalRecognition", "EndFusedChannel", 2, 170); // Set the end channel for the zone
	Config->SetLong("SignalRecognition", "RecognizerID", 2, 2); // Assign a recognizer (by its id in the list of recognizers) to this zone
	Config->AppendTextToSection("SignalRecognition", ""); // Insert an empty line
	
	Config->CreateNewSection("DynamicClustering");
	Config->SetLong("DynamicClustering", "MaxSegmentSize", 2000); // Maximal spatial size of one segment
	Config->SetLong("DynamicClustering", "int_SigmaForSpatialSmoothing", 3); // Stdev of the kernel in the Gaussian smoothing filter 
	Config->SetLong("DynamicClustering", "GUIPrintClusterMode", 4); // Print info to the GUI file. 0: centroids; 1: centroids+(hard decision); 2: centroids+intensity+(hard decision); 3: clusters+intensity; 4: clusters+intensity+(hard decision)
	Config->AppendTextToSection("DynamicClustering", ""); // Insert an empty line
	
	Config->CreateNewSection("TargetTracking");
	Config->SetLong("TargetTracking", "TrackingType", 3); // 0: PDA; 1: IMMPDA; 2: JPDA; 3: IMMJPDA
	Config->SetFloat("TargetTracking", "InitialTrackHalfWidth", 1.0); // Initial half-width of a track 
	Config->SetFloat("TargetTracking", "ObservationHalfWidth", 2.0); // Initial observation area to search the track
	Config->SetFloat("TargetTracking", "StdevOfMeasurementNoise", 3.0); // Stdev of the measurement noise
	Config->SetFloat("TargetTracking", "StdevOfProcessNoise", 0.2); // Stdev of the process noise
	Config->SetFloat("TargetTracking", "DetectionProbability", 0.9); // Detection probability = 1 - (miss probability)
	Config->SetLong("TargetTracking", "bool_DoRemoveStalkers", 1); // Removal of possible stalking tracks is recommended
	Config->SetLong("TargetTracking", "AssociationOption", 2); // Track-to-cluster association type. // 0: weights and variances of the measurement clusters are ignored (all clusters are identical); 1: variances are ignored (all clusters have equal variance); 2: both weight and variance of clusters are taken into account
	Config->AppendTextToSection("TargetTracking", ""); // Insert an empty line
	
	
	Config->SaveToFile("/Data/ConfigFiles/MainConfigExample.txt"); // Name of the output config file
	delete Config;
}





// Example of how to create a config file for recognition purposes (RecInfo)
void CreateNewRecInfoConfig()
{
	CConfigFileWriter * Config = new CConfigFileWriter();
	
	Config->CreateNewSection("CommonParameters");   
	Config->SetName("CommonParameters", "Description", "RecInfo"); // Description of the file (RecInfo)
	Config->SetLong("CommonParameters", "ClassCode", 63); // Set the class code (decimal -> binary), which map enabled classes: 111111 = 63 (here all classes are enabled)
	Config->SetLong("CommonParameters", "TaskType", 0); // 0: multiclass classification, 1: pairwise "1 vs 1" classification; 2 : pairwise "Half vs Half"
	Config->SetLong("CommonParameters", "LocalNumberOfClasses", 6); // Number of classes in this particular classifier (there my be more classes in general, if some classes are not included in this classifier)
	Config->AppendTextToSection("CommonParameters", ""); // Insert an empty line
	
	Config->CreateNewSection("FeatureExtraction");
	Config->SetName("FeatureExtraction", "Method", "WaveletPacket"); // Type of the feature extraction. The following methods will be available soon: "PSD", "ACF"
	Config->SetName("FeatureExtraction", "WaveletTableFile", "/Data/ConfigFiles/WaveletCoefs.txt"); // Path to the file of wavelet coefficients (this parameter only makes sense for Wavelet Packet features)
	Config->SetLong("FeatureExtraction", "UsedWaveletLevels", 6); // Number of wavelet levels used to create energy packets (this parameter only makes sense for Wavelet Packet features)
	Config->SetName("FeatureExtraction", "WaveletType", "Daub"); // WaveletPacket type: Daub, Coiflet (this parameter only makes sense for Wavelet Packet features)
	Config->SetLong("FeatureExtraction", "WaveletFilterLength", 4); // Size of the wavelet filter in accordance to the wavelet type. Note: it cannot be any integer, see the table of wavelet coefficients. (this parameter only makes sense for Wavelet Packet features)
	Config->AppendTextToSection("FeatureExtraction", ""); // Insert an empty line

	Config->CreateNewSection("LinearTransform");
	const char MapFileName[] = "/Data/Maps/bD4(27)_6.0.c13.txt"; // Path to the text file, where the vector or matrix of the linear transform is stored
	FILE * pMapFile = fopen(MapFileName, "r");
	if(pMapFile == NULL)
	{
		printf("\nUnable to open the file %s", MapFileName);
		delete Config;
		return;
	}
	Config->AppendDataFragmentToSection("LinearTransform", pMapFile); // Add a data fragment to the section
	fclose(pMapFile);
	
	Config->CreateNewSection("Classification");
	const char LibFileName[] = "/Data/Libs/FilteredLib_AllkNN(K=9)_New2a(6classes).txt"; // Path to the text file, where the library of the classifier is stored
	const unsigned int K = 7; // Set parameter K for the kNN classifier
	// Note: you need a reference to the project FeatureExtraction to enable these lines
	/*
	Classifier_kNN * A = new Classifier_kNN(K, LibFileName); // Creating a classifier
	FILE * TempFile = tmpfile(); // Temporary file
	A->SaveContentToFile(TempFile); // Information is uploaded to the temporary file
	Config->AppendDataFragmentToSection("Classification", TempFile); // Content of the temporary file is appended to the section "Classification"
	fclose(TempFile);
	delete A;
	*/

	Config->SaveToFile("/Data/ConfigFiles/RecInfoConfigExample.txt"); // Name of the output RecInfo file
	delete Config;
}


#endif /*EXAMPLEOFCONFIGFILE_H_*/
