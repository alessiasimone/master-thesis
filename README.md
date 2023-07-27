# Master Thesis - Smart Healthcare: CPS for Stress Detection
## Project Introduction
Smart Wearable Devices played a crucial role in the industrial revolution, during which Artificial Intelligence and the Internet of Things developed increasingly with the need for collaboration between humans and technology. In particular, the development of nanotechnologies and the usage of sensors previously dedicated to medical fields only led to the increase of the marketplace of intelligent healthcare devices to allow everyone to monitor necessary biometric measures and prevent many dangerous situations. 

However, many scientists are still investigating the proposal of intelligent wearable devices with learning algorithms inside to reconcile the reduced executive capacity of devices with the accuracy of models.

While supervised learning models are presented as a more agile solution, deep learning models usually appear more laborious but more accurate. 
This project compares both solutions to balance model capability and accuracy, aiming to propose a tool to support experts in the field during psychological therapies. 

### Folder Structure
codes/
├─ EDA.ipynb
├─ data_preprocessing.ipynb
├─ preprocessing_wesad.ipynb
├─ ML/
│  ├─ classifier.sav
│  ├─ machine_learning.ipynb
│  ├─ scaler.pkl
├─ DL/
│  ├─ LSTM.ipynb
individual_lag_features/
├─ lags_dataset_0.csv
├─ lags_dataset_1.csv
├─ lags_dataset_2.csv
├─ lags_dataset_3.csv
├─ lags_dataset_4.csv
├─ lags_dataset_5.csv
├─ lags_dataset_6.csv
├─ lags_dataset_7.csv
├─ lags_dataset_8.csv
├─ lags_dataset_9.csv
├─ lags_dataset_10.csv
├─ lags_dataset_11.csv
├─ lags_dataset_12.csv
├─ lags_dataset_13.csv
├─ lags_dataset_14.csv
README.md
lags_dataset.csv
scaled_dataset.csv


## Machine Learning
This project evaluates extracted statistical lag features from the BPM and Skin Temperature signals with an overlapping window of 3 seconds. The models which have been tried out were: Logistic Regression, Gaussian Naive Bayes, Random Forest and Extreme Gradient Boosting. The optimal model regarding the weighted F1 score and score time has been chosen. Halving Grid Search with Leave-One-Subject-Out cross-validation technique has been used to train and validate the models with the optimal hyperparameters.

## Deep Learning
An LSTM on the augmented dataset has been applied as a Deep Learning approach. For this purpose, the BPM and Skin Temperature sequences have been considered by considering only the baseline and stress protocol states. First, a PAR model has been trained to generate synthetic signals by considering the time variable and ID for independent sequences within subjects. Then, a one-layer LSTM was applied with 16 neurons and trained with a Leave-One-Subject-Out cross-validation technique.

## Results
Despite the promising results achieved from the LSTM, the poor number of subjects and the visual inspection of the loss functions throw the topics led to a clear overfitting of the model. Thus, Logistic Regression reached slightly better metrics and has been selected as an optimal model due to its trade-off between F1 score and scoring time. 

## References
https://towardsdatascience.com/quickly-test-multiple-models-a98477476f0


