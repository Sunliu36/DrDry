# Dr.Dry: Neural-Network-Based Clothes Drying Time Prediction System

## Project Overview

Dr.Dry is an innovative intelligent system designed to predict clothes drying time using advanced neural network technology. By integrating environmental sensors and machine learning, the system provides accurate drying time estimates, helping users efficiently manage their laundry process.

## Key Features

- **Real-time Drying Time Prediction**: Estimates clothes drying time within 10 minutes of hanging
- **Weather Integration**: Combines real-time and forecasted weather data
- **Multi-platform Support**: Available on Web and Mobile (Android/iOS)
- **Modular Design**: 3D-printed sliding rails and storage box for easy installation

## System Architecture

| Component | Technology | Function |
|-----------|------------|----------|
| Hardware | ESP32 | Central control unit |
| Sensors | DHT22, HX711, Soil Moisture | Environmental and clothing data collection |
| Frontend | React, Flutter | Web and mobile interfaces |
| Backend | Next.js, Firebase, Nginx | Server-side processing and authentication |
| Database | PostgreSQL (Prisma ORM) | Data storage and management |
| ML Model | Keras, TensorFlow, scikit-learn | Neural network for prediction |

## Prediction Model Details

### Model Specifications
- **Architecture**: 7-layer Neural Network
- **Activation Functions**: 
  - Hidden Layers: ReLU
  - Output Layer: Linear
- **Regularization**: Dropout layers to prevent overfitting

### Performance Metrics
- **Maximum Time Error**: 47 minutes
- **Accuracy within 20 minutes**: 94.07%

### Training Data
- **Dataset Size**: Approximately 35,000 records
- **Split**: 90% training, 10% validation
- **Test Dataset**: 1,000 records

## Prediction Input Features
- 10-minute continuous environmental data
- Humidity and temperature (environment and clothing)
- Clothing weight and initial weight
- Total 52-dimensional feature vector

## Target Prediction
Remaining time to complete drying + 30-minute offset

### Drying Completion Criteria
1. Weight curve slope stabilization
2. Soil moisture sensor conductivity approaching zero

## Target Users

### Individual Users
- Busy professionals
- Apartment and small home residents
- Families with sensitive skin or young children

### Institutional Users
- Universities (dormitories)
- Hotels
- Fitness centers

## Future Development Roadmap

| Phase | Key Objectives |
|-------|----------------|
| Short-term (1 year) | - Model optimization <br> - UI/UX improvements <br> - Weather API integration <br> - Initial market promotion |
| Medium-term (1-3 years) | - Smart device integration <br> - Expanded application scenarios <br> - International market expansion |
| Long-term (3-5 years) | - Smart home ecosystem integration <br> - Cloud-based personalized services <br> - Sustainable product development |

## Environmental Adaptability
Suitable for various settings including:
- Dormitories
- Balconies
- Indoor and outdoor spaces
- Diverse climate conditions

## Sustainability
- Reduces dependency on energy-intensive dryers
- Promotes natural drying
- Potential for ESG (Environmental, Social, Governance) alignment

## Technologies Used
- Neural Networks
- IoT Sensors
- Cloud Computing
- Cross-platform Mobile Development