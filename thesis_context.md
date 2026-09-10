### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

### **“Sarwave: a Support Vector Machine Based Human Presence Detection and Localization Using mmWave Radar for Indoor Fire Search and Rescue Operations”** 

An Undergraduate Thesis   to be 

Submitted to the Faculty of the 

College of Engineering, Computer Studies, and Architecture 

Lyceum of the Philippines University - Cavite 

In Partial Fulfillment of the 

Requirements for the Degree 

Bachelor of Science in Computer Engineering 

**Komiya, Yutaka D.A.** 

**Nadua, Aljin C.** 

**Villamor, Karl Robert C.** 

May 2026 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**||
|---|---|
|**TABLE OF CONTENTS**||
|**TITLE PAGE**<br>|**i**|
|**TABLE OF CONTENTS**|**ii**|
|**TABLE OF FIGURES**|**vii**|
|**LIST OF TABLES**|**viii**|
|**CHAPTER I: INTRODUCTION**|**1**|
|Background of the Study|1|
|Objectives of the Study|3|
|Significance of the Study|5|
|Scope and Limitations|7|
|**CHAPTER II: REVIEW OF RELATED LITERATURE**|**10**|
|Conceptual Literature|10|
|Millimeter-Wave (mmWave) Radar Technology|10|
|Point Cloud Data and Spatial Feature Extraction|11|
|Support Vector Machine (SVM) Classification|12|
|Human Presence Detection Using Radar|13|
|Radar-Based Localization|14|
|Search and Rescue Operations and the Role of Sensor Technology|15|
|Edge Computing for Embedded Sensing Systems|16|
|Related Literature|17|
|Human Vital Signs Detection Methods and Potential Using Radars: A Review|17|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE|ii|



|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|Non-Contact Vital Signs Monitoring Using Millimeter-Wave Radar Technologies 17<br>The Application of FMCW Radar Technology in Human Heartbeat Respiration|
|Monitoring<br>18|
|Influence of Smoke Density on Millimeter-Wave Radar Usability<br>19|
|A Noise Reduction Algorithm for FMCW Radar Vital Signs Signals<br>20|
|Variational Modal Decomposition Based on Whale Optimization Algorithm for Vital|
|Signs Detection of FMCW Radar<br>21<br>Enhanced Vital Sign Monitoring in Multi-Target Environments: A FMCW Radar|
|Approach with Blind Source Separation<br>21|
|Millimeter-wave Radar Vital Signs Detection Based on Modified Independent|
|Component Analysis<br>22|
|Machine Learning in RADAR-Based Physiological Signals Sensing: A Scoping|
|Review<br>23|
|Machine Learning for Healthcare Radars: Recent Progresses in Human Vital Sign|
|Measurement and Activity Recognition<br>23|
|Related Studies<br>24|
|FMCW Radar for Human Detection in Collapsed Structures for Post-Disaster Search|
|and Rescue<br>24|
|Millimeter Wave Radar-Based Survivor Detection and Localization in Indoor Rescue<br>Scenarios<br>25|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE<br>iii|



|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|Detection of Trapped Victims Behind Large Obstacles Using Radar Sensors: A<br>Review<br>26<br>Advancing Remote Life Sensing for Search and Rescue: A Novel Framework for|
|Precise Vital Signs Detection via Airborne UWB Radar<br>26|
|Detection of Vital Signs in Debris Environment using SFCW Radar<br>27|
|Application of Millimeter-Wave Radar in the Life Detection System of the Mining|
|Area<br>27|
|Through the wall human heartbeat detection using single channel CW radar<br>28<br>Breathing Rate Estimation Behind Walls Exploiting a UWB Radar Sensor<br>29|
|Detection and Classification of Human Respiration Under Building Debris Model|
|Using VHF/UHF Waves<br>29|
|Deep Learning Classifier for Robust Artifact Rejection in FMCW Radar Vital Sensing|
|30|
|Related System<br>31|
|mmRH: Noncontact Vital Sign Detection With an FMCW mm-Wave Radar<br>31<br>MmECare: Enabling Fine-grained Vital Sign Monitoring for Emergency Care with|
|Handheld MmWave Radars<br>31<br>Real Time Vital Sign Monitoring System using AWR1642 Radar Module with|
|Remote Access<br>32|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE<br>iv<br>Conceptual Framework<br>33|



|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**||
|---|---|
|Theoretical Framework|34|
|Synthesis|36|
|Definition of Terms|41|
|**CHAPTER III: METHODOLOGY**|**47**|
|Research Design|47|
|Research Locale|47|
|Participants of the Study|48|
|Sampling Techniques|48|
|Research Instrument|49|
|Data Gathering Procedure|50|
|Data Collection Protocol|51|
|Feature Extraction|53|
|Training Strategy|54|
|Cross-Validation and Hyperparameter Tuning|54|
|Project/System Construction Procedure|55|
|System Overview|57|
|Hardware Assembly and Wiring|59|
|Prototype Enclosure Design|60|
|Radar Configuration|60|
|Real-Time Pipeline|60|
|Multiple Constraints Analysis|61|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE|v|



|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**||
|---|---|
|Plan A|62|
|Plan B|66|
|Plan C|71|
|Overall Comparison|75|
|System Model / Project Model|76|
|Sample Size Justification|78|
|Test Scenarios and Procedure|80|
|Project Testing, Evaluation, and Validation|82|
|Baseline Comparison and Statistical Validation|91|
|**REFERENCES**|**93**|



COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

vi 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|



|**TABLE OF FIGURES**|
|---|
|Figure 1_Conceptual Framework of Sarwave_..................................................................... 33|
|Figure 2_Iterative Method Diagram_................................................................................... 56|
|Figure 3_Hardware Assembly_............................................................................................. 59|
|Figure 4_Perspective Model of Plan A_................................**Error! Bookmark not defined.**|
|Figure 5_Perspective Model of Plan B_................................**Error! Bookmark not defined.**|
|Figure 6_Perspective Model of Plan C_...............................**Error! Bookmark not defined.**|
|Figure 7_Functional Block Diagram_.................................................................................. 77|
|Figure 8_Testing Environment_............................................................................................ 81|



COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

vii 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

|**LIST OF TABLES**|
|---|
|Table 1_Analytical Hierarchy Evaluation Rubric_............................................................... 61|
|Table 2_Scoresheet of Plan A_.............................................**Error! Bookmark not defined.**|
|Table 3_Scoresheet of Plan B_.............................................**Error! Bookmark not defined.**|
|Table 4_Scoresheet of Plan C_.............................................**Error! Bookmark not defined.**|
|Table 5_Comparison of Scoresheets of Plan A, B, and C_................................................... 75|
|Table 6_Test Scenario Matrix_............................................................................................. 80|
|Table 7_Performance Metrics and SAR Relevance_............................................................ 82|
|Table 8_Acceptable Thresholds_.......................................................................................... 86|



COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

viii 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

### **CHAPTER I: INTRODUCTION** 

### **Background of the Study** 

Indoor or residential fires are one of the most common and impactful disasters globally (Rahman et al., 2023). Research by Joo and Seo (2025) shows that smoke-induced visibility loss fundamentally alters human evacuation dynamics during indoor fire emergencies. This ultimately leads to people getting trapped in disoriented states where spatial awareness collapses, rendering even familiar floor plans unrecognizable. Therefore, survivors should be located and rescued as fast as possible in a low-visibility area due to debris or dense smoke to avoid casualties. 

The use of electromagnetic waves are already being researched and proposed for search and rescue operations (Michalopoulos et al., 2025). Ultra-Wide Band (UWB) Radar paired with ML algorithms has shown to be effective in human life detection behind walls or debris, achieving more than 95% accuracy (Uzunidis et al., 2024). Thermal imaging with infrared radiation (IR) is also used along with drones to detect people in SAR operations and can track multiple targets (Yeom, 2024). However, these technologies have their limitations, Ultra-Wide Band (UWB) operates at a lower carrier frequency (3.1–10.6 GHz), which provides deeper material penetration but results in lower angular resolution due to the wider beamwidths achievable at compact antenna apertures (Wang et al., 2023). As for infrared-based sensors, their effectiveness declines in high-temperature environments because of low thermal contrast. Furthermore, infrared radiation is unable to penetrate glass, rendering survivors positioned behind windows effectively invisible to thermal 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

1 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>2<br>imaging (Tateoka & Kidera, 2026). Similarly, conventional optical cameras and LiDAR<br>systems fail to provide reliable detection in fire-ravaged environments, as their<br>performance is severely degraded by dense smoke, inconsistent lighting, and extreme<br>thermal fluctuations.<br>To address these specific limitations, specifically the low resolution of UWB and<br>the environmental sensitivity of optical and infrared sensors, millimeter-wave (mmWave)<br>radar has emerged as a promising alternative. It's fine range and Doppler resolution (e.g.<br>sub-10 cm with wide bandwidth) enable detection of human motion or vital signs (Wu et<br>al., 2023). Recent work integrates mmWave data with deep learning (CNNs, RNNs,<br>Transformers) for target detection and classification, augmenting limited radar data via<br>spatiotemporal joint classification framework for millimeter-wave point cloud sequences<br>(Shi & Shi, 2026). A key advantage of mmWave is immunity to visible light occlusion.<br>Numerous studies show that mmWave signals readily penetrate smoke, fog, dust, and even<br>clothing or foliage (Chen et al., 2024), while LiDAR and cameras fail. For instance.<br>Despite the advantages of mmWave technology, a critical challenge remains in the<br>real-time interpretation of radar point clouds within the high-clutter environments of active<br>fire scenes. While previous research has validated the penetration capabilities of mmWave<br>signals, there is a lack of robust, localized frameworks, particularly in the Philippine<br>residential context, that integrate high-resolution radar sensing with lightweight machine<br>learning models for rapid, binary classification of human presence. Consequently, there is|



### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

a need to investigate whether lightweight embedded mmWave radar systems can provide meaningful human-presence sensing capability in smoke-obscured indoor environments as a supplementary tool for first responders. Beyond its immediate technical application, the study bridges the gap between emergency response and global sustainability initiatives, aligning with the United Nations Sustainable Development Goals (SDGs) to foster safer and more resilient societies. By refining radar-based human detection, the research directly supports SDG 11 (Sustainable Cities and Communities), aiming to mitigate the catastrophic impact of fire-related disasters in high-density urban landscapes. This initiative further contributes to SDG 9 (Industry, Innovation, and Infrastructure) by promoting the development of localized, hightech sensing frameworks that enhance domestic disaster-preparedness infrastructure. Ultimately, these technological advancements serve the core objective of SDG 3 (Good Health and Well-being), as rapid, reliable survivor detection is essential to minimizing casualties and reducing the severe, long-term health risks that fire exposure poses to both vulnerable civilians and frontline rescue personnel. **Objectives of the Study** The general objective of Sarwave is to develop an SVM-based human presence detection and localization system using mmWave radar technology designed for indoor fire 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

3 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|search and rescue operations where visual detection is impossible. Specifically, this study<br>aims:<br>1.) To design a single-unit mmWave radar acquisition system capable of generating<br>real-time 3D point clouds, featuring integrated local processing, an onboard visual<br>interface for results, and a high-capacity power management for extended portable<br>operation with development of a companion dashboard application that receives and<br>displays real-time Sarwave detection and localization data wirelessly, providing<br>remote situational awareness during indoor operation.<br>2.) To develop an embedded software system capable of independent, onboard<br>execution, featuring real-time point cloud extraction and analysis, human presence<br>classification via a trained Support Vector Machine, and target localization using<br>DBSCAN-based clustering.<br>3.) To implement the Sarwave system in a controlled, simulated environment by<br>optimizing single-operator usability, ensuring the device fulfills the physical and<br>operational demands of indoor fire search and rescue conditions, including restricted<br>mobility, limited visibility, and the necessity of operating while wearing standard<br>firefighting protective equipment.<br>4.) To test and validate the Sarwave system through structured experiments across<br>varying distances and obstruction conditions, assessing system performance using<br>the following validation criteria such as accuracy, false negative rate, detection<br>reliability, localization acceptability.|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>4|



### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

5.) To evaluate the overall system performance of Sarwave under ISO/IEC 25010 quality characteristics of functional correctness, reliability, and performance efficiency, assessed through processing time and detection latency, and ISO/IEC 18305 localization acceptability criteria, benchmarking the SVM-based classification approach relative to a rule-based point density threshold baseline across accuracy, false positive rate, false negative rate, and F1 score to determine the operational viability of Sarwave as a supplementary human-presence sensing tool for smoke-obscured indoor fire search and rescue. 

### **Significance of the Study** 

The development of a portable device that can detect and locate a human in a visually challenging environment holds profound implications for SAR applications. The findings of this study will be beneficial to the following sectors: **Local Government Units (LGUs).** Consistent with the BFP Modernization Act (RA 11589), this research offers a self-contained and affordable alternative to expensive, imported SAR equipment. This allows LGUs to equip local fire stations with advanced detection technology that fits within municipal budget constraints. 

**Disaster Risk Reduction and Management (DRRM) Offices.** Sarwave provides DRRM offices with a localized sensing tool that strengthens disaster response preparedness 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

5 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|at the community level. By offering a cost-effective and deployable human presence<br>detection system, DRRM offices can integrate Sarwave into their emergency response<br>protocols, enhancing their capacity to coordinate and support fire search and rescue<br>operations within their jurisdictions.|
|**Bureau of Fire Protection (BFP).**This study provides the BFP with a localized,<br>cost-effective technological framework to enhance interior search and rescue operations.<br>By providing a tool that "sees" through smoke and glass, the system assists firefighters in<br>making faster, data-driven decisions, thereby reducing their time spent in high-risk "hot<br>zones.”|
|**Fire Victims.**The primary beneficiaries are individuals trapped in fireravaged<br>structures. By utilizing mmWave radar to overcome zero-visibility conditions, the system<br>demonstrates a technical approach that, if further developed, may contribute to improved<br>victim localization capability in smoke-obscured fire environments (Starnes, 2021).|
|**Future Researchers.**This research serves as a foundational reference for studies<br>involving through-obstacle sensing and human activity recognition in extreme<br>environments. The methodologies established for point cloud feature extraction and binary<br>classification can be expanded for more complex SAR applications, such as multi-target<br>tracking or vital sign monitoring.|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>6|



### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

### **Scope and Limitations** 

This study covers the design, development, and experimental evaluation of a prototype mmWave radar-based human detection system intended for first responder use in indoor structural fire scenarios. The system uses a single TI IWR6843AOPEVM evaluation module, a 60 GHz mmWave sensor platform that provides access to point cloud data via USB interface, paired with a Raspberry Pi 5 as the onboard processing unit. The study encompasses radar hardware configuration using TI's mmWave SDK; point cloud data collection in a controlled laboratory environment simulating fire conditions; SVM model training and validation using locally collected data; and real-time output display via an onsite LCD module. The system targets stationary conscious human survivors, persons who are trapped or disoriented but retain detectable micro-motion including natural breathing and involuntary postural shifts, and operates under a scan-and-hold protocol in which the operator maintains a stationary position during each 2–3 second detection window, eliminating ego-motion Doppler contamination and ensuring reliable feature extraction. Because the SVM classifier is trained and validated on the harder stationary case, mobile victims, who produce stronger and more distinctive Doppler returns, are expected to be detected as a natural consequence of the classifier's learned boundary, though this assumption is not empirically tested and is identified as a direction for future work. The SVM model is scoped to binary classification, human present or human not present, and localization is limited to approximate positional output within the radar's single 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

7 

**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** field of view, not constituting precise coordinate-level tracking. The use case is specifically motivated by indoor search and rescue scenarios where vision-based classification is impossible due to smoke, dust, or low-light conditions, under which mmWave radar remains fully operational (Tateoka & Kidera, 2026). The Sarwave system operates under several constraints that define the boundary conditions of this study. The system is a handheld, manually operated device, requiring a first responder to physically carry and direct the radar unit during each scan-and-hold detection window, meaning system performance is inherently dependent on operator handling and positioning discipline. All data collection and testing are conducted in a controlled indoor environment simulating fire conditions, not in an actual burning structure, and results may not fully generalize to live fire environments with unpredictable heat convection, structural collapse, or extreme RF interference. The system is strictly scoped to indoor use and is not applicable to underwater or outdoor open-field search and rescue scenarios, as the operational assumptions, environmental conditions, and radar propagation characteristics of those environments fall outside the validated scope of this study. The system is designed and validated for a single stationary conscious human target at a time, with multi-target detection and mobile-specific validation outside the validated scope. Using a single radar unit further constrains the detection zone to one field of view, requiring physical repositioning to cover areas outside the sensor's coverage angle. The system relies entirely on the IWR6843AOP's onboard DSP output via processed point clouds, meaning 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

8 

**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

raw I/Q signal data is not used and vital sign monitoring such as breathing rate estimation is outside the system's scope. The dataset combines locally collected IWR6843AOP data with quality-filtered RadIOCD frames as a supplementary source; a domain gap remains between RadIOCD's logistics hall collection environment and this study's fire simulation environment, which is managed through quality filtering and minority weighting but cannot be fully eliminated. Localization output is centroid-based and approximate, intended only to provide responders a general directional indication rather than an exact coordinate. The system outputs result locally via LCD, and remote transmission of detection alerts to an incident commander is outside this study's scope. Finally, the system assumes that human targets exhibit measurable micro-motion such as respiration or involuntary body movement; completely motionless targets with no observable micro-motion may reduce detection reliability due to limited Doppler variation and point cloud activity. 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

9 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

### **CHAPTER II: REVIEW OF RELATED LITERATURE** 

This chapter discusses a review of relevant literature and previous studies that have been collected to provide a deeper understanding of the research topic. A summary of key findings, research works, and related literature is provided in the synthesis section at the conclusion of this chapter. 

### **Conceptual Literature** 

### **_Millimeter-Wave (mmWave) Radar Technology_** 

Millimeter-wave (mmWave) radar refers to radar systems that operate in the frequency range of 30 GHz to 300 GHz, corresponding to wavelengths between 1 mm and 10 mm. These systems transmit electromagnetic waves and analyze the reflected signals to measure the range, velocity, and angular position of objects in the environment. Unlike optical or infrared sensors, mmWave radar is unaffected by environmental conditions such as smoke, dust, darkness, and fog, which makes it particularly well-suited for deployment in hazardous indoor environments where visual sensing is degraded or completely unavailable (Antolinos, 2023). Among the various radar waveform types, Frequency-Modulated Continuous Wave (FMCW) radar is the most widely adopted modality for short-range sensing applications. FMCW radar continuously transmits a signal whose frequency increases linearly over time, known as a chirp, and mixes the transmitted signal with its own reflection to extract range and velocity information simultaneously. The key advantage of FMCW radar over pulsed 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

10 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

radar systems is its ability to achieve high range resolution without requiring high peak transmit power, enabling compact and low-cost hardware implementations. In the proposed Sarwave system, the Texas Instruments IWR6843AOP operates as a 60 GHz FMCW radar sensor, chosen for its combination of high angular resolution, compact form factor, and native point cloud output via USB interface. The IWR6843AOP is a single-chip mmWave sensor integrating the transmit and receive antenna arrays, radio-frequency front end, analog-to-digital converters, and a digital signal processor within one package. The sensor outputs processed point cloud data, a spatial representation of detected objects as a set of three-dimensional coordinate points, without requiring the host system to handle raw I/Q signal data. This point cloud output is the primary data modality used by the Sarwave system for human presence detection and localization. 

### **_Point Cloud Data and Spatial Feature Extraction_** 

A point cloud is a collection of data points defined in three-dimensional space, where each point represents a location at which the radar detected a reflected signal above a noise threshold. In the context of mmWave radar, each point in the cloud carries spatial coordinates (x, y, z), a radial velocity value derived from the Doppler shift of the reflected signal, and a signal-to-noise ratio (SNR) value indicating detection confidence. Together, these attributes form a rich spatial and kinematic representation of the scanned environment. 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

11 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

For human presence detection, point cloud features are extracted to characterize the spatial distribution and motion properties of detected clusters. Key features used in the Sarwave system include point density, the number of points returned within a defined spatial region, centroid position, which represents the mean three-dimensional coordinate of a cluster and serves as the basis for approximate localization, Doppler velocity, which captures micro-motion associated with breathing and minor body movement, and spatial spread, which measures the geometric extent of the point cluster. These features collectively enable a machine learning classifier to distinguish between the point cloud signature of a human target and background clutter such as structural debris, walls, and equipment. 

### **_Support Vector Machine (SVM) Classification_** 

A Support Vector Machine is a supervised machine learning algorithm that performs classification by finding the optimal hyperplane that separates data points belonging to different classes in a high-dimensional feature space. The SVM maximizes the margin between the nearest data points of each class, referred to as support vectors, and the separating hyperplane, which yields strong generalization performance on unseen data even when the training dataset is relatively small. This property is particularly advantageous for embedded sensing applications where data collection is constrained by testing time and manpower. 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

12 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|For non-linearly separable data, SVMs employ kernel functions, mathematical<br>transformations that map input features into a higher-dimensional space where linear<br>separation becomes feasible. Common kernel functions include the Radial Basis Function<br>(RBF), polynomial, and sigmoid kernels. In the Sarwave system, the SVM is configured<br>for binary classification: the positive class corresponds to human presence, and the negative<br>class corresponds to the absence of a human target, encompassing background clutter,<br>debris vibration, and environmental noise. The choice of SVM over deep learning<br>classifiers is motivated by its computational efficiency, interpretability, and demonstrated<br>suitability for radar-based classification tasks on resource-constrained hardware (Nocera et<br>al., 2024; Ali et al., 2024).|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>13<br>**_Human Presence Detection Using Radar_**<br>Human presence detection refers to the task of determining whether a human being<br>is located within a sensor's field of view based on signals reflected from the body. Unlike<br>activity recognition or vital sign monitoring, presence detection is a binary classification<br>problem that requires only a confident determination of occupancy, human present or not<br>present, without characterizing the person's specific activity, physiological state, or precise<br>coordinates. This makes it a computationally tractable problem well-matched to the<br>processing capabilities of embedded edge hardware such as the Raspberry Pi 5.<br>In radar-based presence detection, the primary physical cues exploited are<br>microDoppler signatures arising from subtle body movements such as breathing,|



|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|involuntary muscle tremors, and slight postural shifts. Research suggests that minimally<br>moving individuals may produce detectable micro-motion in radar returns. The Sarwave<br>system is designed to exploit this property, though detection reliability for fully motionless<br>subjects remains a boundary condition requiring further investigation. This is especially<br>critical in fire search and rescue scenarios where conscious victims may be trapped or<br>disoriented and unable to call for help. The Sarwave system targets stationary conscious<br>victims as the primary worst-case detection scenario under a hold-and-search operational<br>protocol. Mobile victims, producing stronger Doppler signatures, are expected to be<br>detected as a natural extension of the stationary-case classifier boundary without requiring<br>separate validation.|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>14<br>**_Radar-Based Localization_**<br>Localization refers to the estimation of a target's position within a defined<br>coordinate frame. In the context of single-unit mmWave radar systems, localization is<br>achieved through the analysis of the range, azimuth angle, and elevation angle data encoded<br>in the point cloud output. Range is derived from the time delay of the reflected chirp;<br>azimuth and elevation angles are estimated from the phase differences between signals<br>received at spatially separated antenna elements, a technique known as angle-of-arrival<br>estimation.<br>The Sarwave system implements centroid-based approximate localization, in which<br>the spatial mean of a detected human point cluster is computed and reported as the|



|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|estimated target position. This approach provides a general directional indication of the<br>victim's location within the sensor's field of view rather than a precise coordinate-level fix.<br>The output is intended to guide first responders toward the victim's approximate location,<br>reducing search time in smoke-obscured environments where visual navigation is severely<br>impaired. The localization output is displayed on an onboard LCD module without<br>requiring any external computing device or wireless transmission, consistent with the|
|system's design for autonomous field operation.|



**_Search and Rescue Operations and the Role of Sensor Technology_** Search and rescue (SAR) operations encompass the organized efforts to locate, access, stabilize, and transport victims of disasters including structural fires, building collapses, earthquakes, and typhoons. In indoor fire SAR scenarios specifically, first responders face conditions that severely degrade conventional search methods: dense smoke eliminates line-of-sight visibility, high temperatures limit safe dwell time, structural instability restricts movement, and acoustic noise from burning materials masks auditory cues. These conditions create a strong operational need for sensor systems capable of detecting human presence through environmental obstructions without requiring physical contact or line-of-sight. The Philippines presents a particularly high-risk national context for structural fire SAR due to its dense informal settlement patterns, prevalence of lightweight construction materials, and the compounding effects of typhoon damage on structural integrity. These 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

15 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

characteristics mean that fire SAR scenarios in the Philippine context are likely to involve debris environments composed primarily of wood, plaster, and fabric, non-metallic lightweight materials through which 60 GHz mmWave radar maintains adequate penetration performance (Uzunidis et al., 2023; Yılmaz et al., 2024). The Sarwave system is specifically scoped to these material conditions, acknowledging the penetration limitations of high-frequency radar through reinforced concrete as a boundary constraint of the system's operational envelope. 

### **_Edge Computing for Embedded Sensing Systems_** 

Edge computing refers to the paradigm of performing data processing and inference locally at or near the data source, rather than transmitting raw data to a remote server or cloud platform. In the context of portable sensing systems, edge computing enables realtime output with low latency, eliminates dependence on wireless network connectivity, and reduces the system's operational footprint to a self-contained unit. These properties are essential for field-deployable SAR devices, which must operate reliably in environments where network infrastructure may be damaged or unavailable. The Sarwave system implements its complete signal processing and classification pipeline on a Raspberry Pi 5 single-board computer, which serves as the edge processing unit. The Raspberry Pi 5 provides sufficient computational resources to receive point cloud data from the IWR6843AOP via USB, extract spatial features, execute real-time SVM inference, and drive an LCD display output, all within a compact, low-power hardware COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

16 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|package. This edge-first architecture is consistent with the system's design objective of<br>producing a self-contained, first-responder-operable device that does not require external<br>computing infrastructure during deployment.|
|**Related Literature**<br>**_Human Vital Signs Detection Methods and Potential Using Radars: A Review_**<br>Kebe et al. (2020) presents a comprehensive review of radar-based vital signs<br>detection methods, covering Doppler radar, FMCW radar, and UWB radar across various<br>frequency ranges and system configurations. The study identifies that while radar-based<br>monitoring offers significant advantages over contact-based methods, including<br>noninvasiveness, privacy preservation, and the ability to detect through obstacles, there<br>remains a lack of standardized evaluation frameworks that compare systems under realistic<br>environmental conditions. The authors highlight that most existing systems are validated<br>in clean laboratory settings with minimal clutter, leaving a significant gap in performance<br>evaluation under cluttered or obstructed environments. This review serves as the<br>foundational taxonomy for the proposed study, establishing the theoretical basis for<br>selecting FMCW mmWave radar as the sensing modality for SAR victim detection in<br>debris environments.|
|**_Non-Contact Vital Signs Monitoring Using Millimeter-Wave Radar Technologies_**<br>Antolinos (2023) surveys the current state of millimeter-wave radar for non-contact<br>vital signs monitoring, examining systems operating between 24GHz and 77GHz across|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>17|



|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|healthcare, elderly care, and emergency applications. The study identifies that while<br>mmWave radar shows strong potential as a non-invasive monitoring solution, most<br>deployed systems require line-of-sight and perform poorly when the subject is partially<br>occluded or positioned at non-frontal angles. The authors recommend that future systems<br>incorporate multi-angle radar configurations and advanced clutter suppression to improve<br>real-world applicability. This paper directly informs the proposed system's sensor<br>positioning strategy, confirming that the IWR6843AOP operating at 60GHz is a suitable<br>hardware choice for life sign detection while also highlighting the clutter suppression<br>challenge that the proposed signal processing pipeline must address.|
|**_The Application of FMCW Radar Technology in Human Heartbeat Respiration_**<br>**_Monitoring_**<br>Tang (2024) investigates the application of FMCW radar for simultaneous heartbeat<br>and respiration monitoring, examining signal extraction techniques including range-FFT,<br>Doppler processing, and phase unwrapping. The study identifies harmonic interference<br>between heartbeat and respiration signals as the primary challenge in FMCW-based vital<br>signs detection and demonstrates that even in low-SNR environments with high<br>interference, conditions directly analogous to a debris-filled SAR scenario, FMCW radar<br>retains sufficient micro-motion signal fidelity to distinguish physiological activity from<br>background noise. This finding supports a foundational assumption of the Sarwave system:<br>that a stationary or unconscious human target produces a Doppler velocity signature<br>detectable above the noise floor of a cluttered indoor environment, providing physical|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>18|



### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

justification for the discriminative value of the Doppler velocity spread feature used in SVM classification. 

### **_Influence of Smoke Density on Millimeter-Wave Radar Usability_** 

In a recent study, Francesca Schenkel and colleagues investigated the use of frequency-modulated continuous-wave (FMCW) millimeter-wave radar for smoke detection and combustion analysis in smoke-obscured environments. Rather than directly measuring smoke density alone, the study evaluated how smoke alters the dielectric properties of air and consequently affects radar signal propagation through phase variations. A 70–90 GHz FMCW radar sensor was used and compared against the MIREX optical extinction system, which served as a reference for smoke concentration measurements. In controlled laminar smoke experiments, smoke density was increased linearly over 550 s, reaching a maximum particle volume fraction of 0.46512 ppm, while radar measurements showed corresponding increases in phase values as smoke concentration increased. The study reported that a radar phase change of 0.4° corresponded to an extinction value increase of 1.938 dB/m, a smoke particle volume fraction increase of 0.09624 ppm, and a refractivity change of 4.9, demonstrating measurable sensitivity of the radar to smokerelated dielectric changes. Furthermore, the authors explained that smoke density was quantified through the optical extinction coefficient, indicating that smoke-induced changes in refractive index alter the signal propagation path and produce detectable radar phase shifts. These findings demonstrated that smoke density affects the radar signal, but it does 

### COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

19 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|not significantly reduce the usability of mmWave radar. Instead, smoke changes the radar<br>measurements in a way that the radar can detect and exploit for smoke characterization. In<br>other words, smoke becomes a measurable phenomenon rather than something that blinds<br>the sensor.|
|**_A Noise Reduction Algorithm for FMCW Radar Vital Signs Signals_**<br>Xie et al. (2026) proposes a dedicated noise reduction algorithm for FMCW radar<br>vital signs signals, addressing the challenge of separating weak physiological motion<br>signatures from environmental noise and hardware-induced interference. The authors<br>identify that standard filtering alone is insufficient for reliable extraction of weak motion<br>signatures in environments with multiple noise sources, and demonstrate that physiological<br>micro-motion remains recoverable even under high-interference conditions when<br>appropriate noise suppression is applied. This finding is relevant to the Sarwave system in<br>establishing that the weak Doppler signatures produced by a minimally moving or<br>unconscious human target are physically separable from environmental clutter, a condition<br>the Sarwave system addresses at the hardware level through the IWR6843AOP's onboard<br>DSP, which performs CFAR detection and clutter suppression before point cloud data is<br>passed to the Raspberry Pi 5. Xie et al. thus supports the feasibility argument that<br>meaningful micro-motion features survive the sensor's preprocessing chain and remain<br>discriminative for SVM-based binary classification in a noisy SAR environment.|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>20|



|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**<br> COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>21<br>**_Variational Modal Decomposition Based on Whale Optimization Algorithm for Vital_**<br>**_Signs Detection of FMCW Radar_**<br>Zheng et al. (2025) applies variational modal decomposition optimized by a whale<br>optimization algorithm to separate breathing and heartbeat components from FMCW radar<br>signals contaminated by body motion and environmental clutter. The authors identify that<br>fixed-parameter decomposition methods fail to adapt to inter-subject physiological<br>variability, proposing an optimization-driven approach that automatically tunes<br>decomposition parameters for each signal segment. The key finding relevant to the<br>proposed study is that physiological micro-motion signatures in FMCW radar returns vary<br>substantially across subjects, postures, and states of consciousness. Meaning a human<br>target's radar reflection cannot be assumed to follow a uniform pattern. This directly<br>informs the data collection protocol of the Sarwave system, specifically the requirement to<br>record samples across multiple human subjects and varied positions rather than training on<br>a single subject profile, ensuring that the SVM classifier generalizes to the irregular or<br>suppressed micro-motion signatures characteristic of distressed or unconscious fire victims.<br>**_Enhanced Vital Sign Monitoring in Multi-Target Environments: A FMCW Radar_**<br>**_Approach with Blind Source Separation_**<br>Yin et al. (2024) applies blind source separation techniques to FMCW radar signals<br>in multi-person environments, enabling independent vital sign extraction for each detected<br>subject without requiring prior knowledge of their number or positions. The authors identify<br>that in multi-target scenarios, radar returns from different individuals overlap significantly,|
|---|



**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** and that achieving real-time separation on resource-constrained hardware remains an unsolved challenge. This finding directly contextualizes a deliberate design constraint of the Sarwave system: single-target detection is not merely a simplification but a principled scoping decision, as the computational and algorithmic complexity required for reliable multi-target separation exceeds what is feasible on a Raspberry Pi 5 within the real-time latency requirements of a field-deployable SAR device. Yin et al. thus establishes the technical boundary that defines Sarwave's current scope while simultaneously identifying multi-target detection as a well-motivated direction for future work. **_Millimeter-wave Radar Vital Signs Detection Based on Modified Independent Component Analysis_** Liu et al. (2023) applies a modified independent component analysis algorithm to millimeter-wave radar signals to separate respiration and heartbeat components that are typically mixed in the raw phase signal. The authors identify that human physiological motion signatures and background interference occupy overlapping statistical spaces in FMCW radar returns, making separation non-trivial even with advanced decomposition methods. This finding is relevant to the Sarwave system in establishing the nature of the classification challenge: in active rescue environments where structural vibrations and equipment movement generate Doppler returns that partially overlap with human micromotion signatures, a fixed-threshold rule-based classifier is inherently limited. Liu et al. thus provides contextual justification for Sarwave's choice of SVM over a simple density 

### COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

22 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|or velocity threshold baseline, as a learned decision boundary is better equipped to separate<br>human micro-motion from mechanically-induced clutter in the overlapping Doppler space<br>characteristic of active fire rescue environments.|
|**_Machine Learning in RADAR-Based Physiological Signals Sensing: A Scoping Review_**<br>Nocera et al. (2024) systematically examines machine learning approaches applied<br>to radar-based physiological signal sensing in a scoping review, covering SVM, Random<br>Forest, CNN, LSTM, and hybrid architectures across 80+ published studies. The authors<br>identify that while deep learning models achieve the highest reported accuracies, they<br>require large training datasets and significant computational resources that are incompatible<br>with edge deployment, a critical constraint for the proposed handheld SAR device. The<br>review further identifies SVM and Random Forest as the most computationally efficient<br>classifiers for radar vital signs tasks, with accuracy competitive with deep learning when<br>feature engineering is carefully designed. This review directly justifies the proposed<br>system's choice of SVM over deep learning, providing a comprehensive evidence base that<br>supports the feasibility and academic defensibility of the SVM approach for a resource-<br>constrained Raspberry Pi deployment.|
|**_Machine Learning for Healthcare Radars: Recent Progresses in Human Vital Sign_**<br>**_Measurement and Activity Recognition_**<br>Ahmed and Cho (2023) examine recent progress in applying machine learning to<br>healthcare radar systems, covering vital sign measurement, fall detection, activity|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>23|



### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

recognition, and anomaly detection across multiple radar modalities. The authors identify that the field is transitioning from rule-based signal processing toward learned representations, with ML models showing particular strength in handling inter-subject variability that degrades fixed-threshold systems. The research gap identified is the lack of standardized benchmark datasets for healthcare radar ML, each study uses proprietary data, making cross-study comparison difficult. This review is cited in the proposed study to contextualize the AI methodology within the broader healthcare radar ML landscape, .and to acknowledge that the use of a publicly available radar dataset (RadIOCD, 2024) as a supplementary data source partially addresses the benchmark dataset standardization gap identified in this review. 

|**Related Studies**|
|---|
|**_FMCW Radar for Human Detection in Collapsed Structures for Post-Disaster Search_**<br>**_and Rescue_**|
|Abdelhamid et al. (2025) directly addresses the application of FMCW radar for<br>detecting human presence inside collapsed building structures following disaster events.<br>The authors identify that existing SAR technologies, including acoustic sensors, search<br>dogs, and optical cameras, are rendered ineffective in collapsed structure environments due<br>to darkness, smoke, dust, and physical obstructions. The study demonstrates that FMCW<br>radar can detect breathing signatures through non-metallic rubble materials including<br>drywall, wood, and fabric, with detection accuracy degrading predictably as debris<br>thickness increases. The research gap identified is the lack of standardized debris|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE|



24 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

penetration performance data for FMCW radar systems under realistic collapsed structure conditions. This paper is the most directly relevant SAR study to the proposed system, providing empirical evidence that FMCW radar, the same type used in the proposed IWR6843AOP sensor, is viable for collapsed structure victim detection in post-disaster scenarios. 

**_Millimeter Wave Radar-Based Survivor Detection and Localization in Indoor Rescue Scenarios_** Tateoka and Kidera (2026) propose a survivor detection and localization system using millimeter-wave radar specifically designed for indoor rescue environments including fire, structural collapse, and smoke-filled spaces. The authors identify that conventional rescue tools cannot simultaneously detect, localize, and track multiple victims in cluttered indoor environments without line-of-sight. By combining range-Doppler processing with spatial clustering algorithms, the system achieves simultaneous detection and approximate localization of stationary victims through non-metallic obstacles. The research gap addressed is the absence of systems that combine presence detection with spatial localization in a single compact radar unit. This paper informs the localization component of the proposed SAR system, specifically supporting the inclusion of approximate victim distance estimation as a secondary output alongside breathing rate detection. 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

25 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|**_Detection of Trapped Victims Behind Large Obstacles Using Radar Sensors: A Review_**<br>Uzunidis et al. (2023) systematically examines radar-based technologies for<br>detecting trapped victims behind large physical obstacles, covering UWB, FMCW, and<br>Doppler radar approaches across frequency ranges from 400MHz to 77GHz. The authors<br>identify that while higher frequency radars offer better range resolution, lower frequency<br>systems demonstrate superior penetration through dense materials such as concrete and<br>masonry. The study identifies a critical research gap: no existing system effectively<br>balances penetration depth, breathing detection sensitivity, and compact hardware form<br>factor for practical field deployment by first responders. This review directly validates the<br>proposed study's technology selection rationale, confirming that 60GHz FMCW radar<br>represents an appropriate balance between detection sensitivity and practical deployability<br>for the lightweight, portable SAR device proposed in this thesis.|
|**_Advancing Remote Life Sensing for Search and Rescue: A Novel Framework for_**<br>**_Precise Vital Signs Detection via Airborne UWB Radar_**<br>Jing et al. (2025) proposes a UAV-mounted UWB radar framework for remote vital<br>signs detection in SAR operations, addressing the limitation that ground-based radar<br>systems cannot access victims in structurally unstable or inaccessible collapse zones. The<br>authors demonstrate that airborne radar deployment significantly expands the search<br>coverage area per unit time compared to manually operated ground systems, while<br>maintaining sufficient signal quality for breathing rate estimation. The research gap<br>identified is the lack of motion compensation algorithms that account for UAV platform|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>26|



|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|vibration, a noise source that masks the subtle breathing signatures of trapped victims.<br>While the proposed thesis focuses on a handheld ground-based system rather than UAV<br>deployment, this paper is cited to acknowledge the broader SAR technology landscape and<br>to justify the handheld form factor as a complementary close-range tool to airborne<br>systems.|
|**_Detection of Vital Signs in Debris Environment using SFCW Radar_**<br>Yilmaz et al. (2024) specifically evaluates Stepped-Frequency Continuous Wave<br>radar for vital signs detection in simulated debris environments, testing through<br>combinations of wood panels, concrete blocks, and fabric layers. The authors identify that<br>debris composition significantly affects signal attenuation and that breathing detection<br>remains feasible through non-metallic debris up to approximately 30cm thickness. The<br>research gap addressed is the absence of standardized debris simulation protocols for<br>evaluating through-obstacle radar performance, most existing studies test through single-<br>material walls rather than mixed-material debris piles. This paper is particularly valuable<br>for the proposed study's experimental design, providing a debris simulation methodology<br>that the proposed system can adapt for its own validation testing using simulated typhoon<br>rubble conditions.<br>**_Application of Millimeter-Wave Radar in the Life Detection System of the Mining Area_**<br>Zhang et al. (2024) applies millimeter-wave radar to life detection in mining<br>accident scenarios, where trapped miners must be detected through rubble, dust, and|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>27|



**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** structural debris following tunnel collapses. The authors identify that mining rescue presents conditions nearly identical to post-earthquake SAR operations, total darkness, extreme dust, non-metallic debris, and unconscious victims, making it a highly relevant analog for the proposed Philippine SAR application. The study demonstrates detection of breathing signatures through mixed rubble materials at ranges up to 3 meters, with accuracy sufficient for rescue prioritization. The research gap identified is the lack of systems optimized for the specific debris composition of mining environments. This paper strengthens the proposed study's significance by demonstrating that mmWave radar life detection has been validated in multiple industrial disaster scenarios, establishing a precedent for the proposed Philippine disaster context application. **_Through the wall human heartbeat detection using single channel CW radar_** Pramanik and Islam (2024) demonstrate that a single-channel continuous wave radar can detect human heartbeat signatures through a standard drywall partition, achieving detection at distances up to 2 meters with a signal processing pipeline based on phase demodulation and bandpass filtering. The authors identify that multi-channel systems, while more accurate, introduce hardware complexity and cost that limits practical deployment in field conditions. The research gap addressed is the minimum hardware configuration required for reliable through-wall vital signs detection, a question directly relevant to the proposed system's design philosophy of using a single compact radar sensor. 

|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE|
|---|



28 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|This paper validates the proposed study's single-sensor approach, confirming that a single<br>IWR6843AOP unit is sufficient for through-debris breathing detection without requiring<br>complex multichannel configurations.|
|**_Breathing Rate Estimation Behind Walls Exploiting a UWB Radar Sensor_**<br>Uzunidis et al. (2024) evaluates UWB radar for breathing rate estimation through<br>wall partitions of varying material composition including drywall, brick, and reinforced<br>concrete, quantifying the degradation in detection accuracy as wall density increases. The<br>authors establish that breathing detection remains highly reliable through lightweight non-<br>metallic materials but degrades significantly through dense materials such as brick and<br>reinforced concrete above 20cm thickness. The research gap identified is the absence of<br>material-specific performance models that would allow operators to predict detection<br>reliability based on known obstacle composition. This paper directly informs the proposed<br>system's scope limitation, validating the decision to test through nonmetallic lightweight<br>debris representative of Philippine typhoon-damaged structures rather than reinforced<br>concrete.|
|**_Detection and Classification of Human Respiration Under Building Debris Model_**<br>**_Using VHF/UHF Waves_**<br>Niyaz et al. (2024) applies VHF and UHF frequency radar to detect and classify<br>human respiration patterns under a physical building debris model constructed from wood,<br>plaster, and rubble materials. The authors compare detection performance across multiple|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>29|



|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>30<br>frequency bands and identify that lower frequencies achieve better penetration at the cost<br>of reduced spatial resolution, while higher frequencies offer precise detection with limited<br>debris penetration. The research gap addressed is the lack of multi-frequency comparative<br>data under realistic debris conditions. This paper is directly relevant to the proposed study's<br>frequency selection rationale, providing empirical data that supports the use of 60GHz<br>mmWave radar for shallow debris scenarios while acknowledging the penetration<br>limitations that define the system's operational scope.<br>**_Deep Learning Classifier for Robust Artifact Rejection in FMCW Radar Vital Sensing_**<br>Fusco et al. (2024) proposes a deep learning classifier specifically designed to reject<br>artifacts in FMCW radar vital sensing caused by body movement, environmental vibration,<br>and hardware noise. The authors identify that artifact contamination is one of the primary<br>sources of false positives in radar-based life sign detection systems, a critical concern for<br>SAR applications where a false positive could misdirect rescue resources. The research gap<br>addressed is the absence of dedicated artifact rejection stages in existing radar vital signs<br>pipelines, with most systems relying on manual data cleaning rather than automated<br>rejection. This paper informs the proposed system's SVM training strategy, specifically<br>supporting the inclusion of negative class examples (noise, debris vibration, no victim)<br>alongside positive class examples (breathing victim) to train a robust binary classifier that<br>minimizes false positives in realistic SAR conditions.|



### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

### **Related System** 

### **_mmRH: Noncontact Vital Sign Detection With an FMCW mm-Wave Radar_** 

Liu et al. (2023) develops a noncontact vital sign detection system using FMCW millimeter-wave radar that simultaneously estimates breathing rate and heart rate without physical contact. The study addresses the limitation of traditional Doppler radar systems that require precise alignment and fail under multipath interference. By implementing a phase extraction algorithm combined with harmonic identification, the system achieves robust detection across multiple subjects at varying distances. The research gap identified is the lack of real-time multi-vital-sign extraction in a single compact FMCW system without external reference sensors. This paper is particularly relevant to the proposed SAR system as it demonstrates that FMCW radar at close proximity can reliably extract breathing rate, the primary life sign indicator the proposed system uses to confirm victim presence in debris. **_MmECare: Enabling Fine-grained Vital Sign Monitoring for Emergency Care with Handheld MmWave Radars_** Chang et al. (2024) presents MmECare, a handheld mmWave radar system designed for emergency care scenarios that enables fine-grained vital sign monitoring including breathing rate, heart rate, and body movement detection at close range. The authors identify that existing mmWave radar systems are designed for fixed installation and cannot be rapidly deployed by first responders in dynamic emergency environments. By designing a compact handheld form factor with real-time signal processing, the system demonstrates 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

31 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|that mmWave radar can be practically deployed by non-specialist rescue personnel with<br>minimal setup time. This paper is the closest existing work to the proposed system in terms<br>of form factor and deployment context, directly supporting the proposed handheld SAR<br>device design and confirming that real-time mmWave vital signs processing is feasible on<br>compact edge hardware.|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>32<br>**_Real Time Vital Sign Monitoring System using AWR1642 Radar Module with Remote_**<br>**_Access_**<br>Srihari et al. (2022) implements a real-time vital signs monitoring system using the<br>Texas Instruments AWR1642 radar module, a close hardware relative to the proposed<br>IWR6843AOP, with remote data access via a local network interface. The authors identify<br>that while radar vital signs research has advanced significantly, the translation of laboratory<br>systems into field-deployable prototypes remains underexplored, with most published<br>systems requiring desktop computers and controlled environments. By implementing the<br>complete signal processing pipeline on embedded hardware with a compact form factor and<br>real-time output display, the system demonstrates the practical feasibility of edgedeployed<br>radar vital signs detection. This paper is directly relevant to the proposed system's hardware<br>implementation, providing a reference architecture for TI mmWave radar embedded<br>deployment and confirming that real-time processing of radar breathing data is achievable<br>on resource-constrained hardware comparable to the Raspberry Pi 5.|



### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

### **Conceptual Framework** 

### **Figure 1** 

### _Conceptual Framework of Sarwave_ 



<!-- Start of picture text -->
OUTPUT<br>Sarwave: an<br>SVM-Based<br>Human<br>Presence<br>Detection and<br>Localization<br>Using<br>mmWave<br>Radar for<br>Search and<br>Rescue<br>Applications<br><!-- End of picture text -->



<!-- Start of picture text -->
INPUT<br>Knowledge Requirement<br> • mmWave radar<br>technology<br> • Point cloud processing<br> • Machine learning (SVM)<br> • Embedded systems design<br> • Search & rescue<br>operations<br> • Edge computing<br> • Signal processing<br>(FMCW)<br>Hardware Requirement<br> • TI IWR6843AOPEVM<br> • Raspberry Pi 5<br> • I2C LCD module<br> • 20,000 mAh power bank<br> • Enclosure / housing<br>Software Requirement<br> • Python 3.11<br> • scikit-learn (SVM)<br> • NumPy / SciPy<br> • DBSCAN (clustering)<br> • TI mmWave SDK<br> • RPLCD (LCD driver)<br> • Raspberry Pi OS (64-bit)<br><!-- End of picture text -->



<!-- Start of picture text -->
PROCESS<br>System Development<br>• Point Cloud Data Processing<br>and Clutter Removal<br>• DBSCAN Clustering and<br>Target Isolation<br>• SVM Binary Classification<br>• Centroid-Based Target<br>Localization<br>• Data Collection and Labeling<br>• Feature Extraction and<br>Engineering<br>• SVM Model Training and<br>Validation<br>• Real-Time LCD Output<br>Display<br>• External Dashboard Display<br>Hardware Development<br> • Hardware Assembly and<br>   Wiring<br> • Radar Configuration and<br>   Setup<br> • Prototype Enclosure<br>   Design<br>Testing, Evaluation, and<br>Validation<br>• Multi-Scenario Validation<br>Testing<br>• Centroid-Based Localization<br>Validation<br>• ISO/IEC 25010 Standards<br>Evaluation<br>• Statistical Evaluation<br><!-- End of picture text -->

|**INPUT**|**PROCESS**|**OUTPUT**|
|---|---|---|
|Knowledge Requirement|System Development||
|• mmWave radar<br>|• Point Cloud Data Processing||
|technology|and Clutter Removal||
|• Point cloud processing|• DBSCAN Clustering and||
|• Machine learning (SVM)|Target Isolation||
|• Embedded systems design|• SVM Binary Classification|Sarwave: an|
|• Search & rescue<br>operations|• Centroid-Based Target<br>Localization|<br>SVM-Based<br>|
|• Edge computing|• Data Collection and Labeling|Human<br>|
|• Signal processing|• Feature Extraction and|Presence|
|(FMCW)|Engineering|Detection and|
||• SVM Model Training and|Localization|
|Hardware Requirement|Validation|Using|
|• TI IWR6843AOPEVM|• Real-Time LCD Output<br>|mmWave|
|• Raspberry Pi 5|Display|Radar for|
|• I2C LCD module|• External Dashboard Display|Search and|
|• 20,000 mAh power bank||Rescue|
|• Enclosure / housing|Hardware Development<br>• Hardware Assembly and|<br>Applications|
|Software Requirement|Wiring||
|• Python 3.11|• Radar Configuration and<br>||
|• scikit-learn (SVM)|Setup||
|• NumPy / SciPy|• Prototype Enclosure||
|• DBSCAN (clustering)|Design||
|• TI mmWave SDK|||
|• RPLCD (LCD driver)|Testing, Evaluation, and||
|• Raspberry Pi OS (64-bit)|Validation||
||• Multi-Scenario Validation<br>Testing||
||• Centroid-Based Localization<br>Validation||
||• ISO/IEC 25010 Standards||
||Evaluation||
||• Statistical Evaluation||
|||**Evaluation**|





<!-- Start of picture text -->
Evaluation<br><!-- End of picture text -->

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

33 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

The IPO Diagram represents the core operational logic of this research, moving beyond a simple linear data flow to an intelligent detection system as illustrated in Figure 1. The framework is designed to address the challenges of locating human survivors in indoor fire environments where smoke, debris, and zero-visibility conditions render conventional detection methods ineffective by structuring the development of Sarwave through a systematic Input-Process-Output (IPO) approach. 

### **Theoretical Framework** 

The Sarwave system is grounded in three converging bodies of scientific evidence that collectively justify its core design decisions. The first concerns radar wave propagation and physiological target response: Abdelhamid et al. (2025) and Yilmaz et al. (2024) confirm that 60 GHz FMCW radar penetrates non-metallic debris typical of Philippine fire disaster environments, while Tang (2024) and Xie et al. (2026) establish that respiratory micro-Doppler signatures remain spectrally recoverable under low-SNR, smoke-laden conditions. Tateoka and Kidera (2026) further validate that combining range-Doppler processing with DBSCAN spatial clustering enables concurrent presence detection and approximate localization through indoor obstructions, the exact two-stage pipeline architecture employed by Sarwave. The second body of evidence concerns embedded edge computation: Srihari et al. (2022) demonstrate that complete FMCW processing pipelines 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

34 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

|are executable on resource-constrained single-board computers, Chang et al. (2024)|
|---|
|validate the operational viability of handheld radar form factors for non-specialist rescue|
|personnel, and Yin et al. (2024) demonstrated the single-target scan-and-hold constraint by|
|showing that multi-target blind source separation violates real-time edge processing|
|budgets.|
|The third body of evidence establishes the statistical necessity of an SVM classifier|
|over both deep learning alternatives and rule-based thresholding. Nocera et al. (2024)|
|identify SVM as the most computationally efficient classifier for edge-deployed radar tasks,|
|producing inference latencies of 1–5 milliseconds on the Raspberry Pi 5 platform, while|
|Liu et al. (2023) demonstrate that human micro-Doppler signatures and fire-environment|
|clutter occupy overlapping spectral spaces, rendering fixed thresholds incapable of reliable|
|discrimination without unacceptable false negative rates. Zheng et al. (2025) and Fusco et|
|al. (2024) further inform the training dataset strategy, establishing that classifier robustness|
|requires both multi-subject, multi-posture positive class diversity and explicit negative class|
|representation of fire-environment clutter returns. Together, these three pillars establish that|
|every architectural decision in Sarwave, from sensor frequency and obstruction scope to|
|edge processing substrate, to the choice of SVM over deep learning, is traceable to peer-|
|reviewed empirical findings rather than arbitrary design preference.|



COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

35 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|**Synthesis** <br>The reviewed literature collectively establishes a strong and convergent evidence<br>base for the feasibility and significance of the proposed Sarwave system. Across the six<br>thematic categories examined, core radar technology, search and rescue applications,<br>through-wall and debris penetration, signal processing, AI and machine learning, and<br>system deployment, a consistent set of findings emerges that directly informs the design<br>decisions, scope boundaries, and academic positioning of this study.<br>The foundational technology literature confirms that FMCW millimeter-wave radar<br>is a well-validated modality for non-contact human sensing. Kebe et al. (2020), Antolinos<br>(2023), and Tang (2024) collectively establish that mmWave radar can detect physiological<br>motion signatures without physical contact, is unaffected by optical obstructions such as<br>smoke and darkness, and operates effectively across a range of distances and subject<br>postures. However, a recurring gap identified across these studies is the predominant<br>evaluation of radar systems in clean, controlled laboratory environments, a condition that<br>does not reflect the cluttered, debris-filled conditions of real SAR deployments. This gap<br>directly motivates the Sarwave system's experimental design, which incorporates<br>controlled simulations of smoke, debris, and limited visibility rather than open room<br>testing.<br>The SAR application literature reinforces the operational need for a sensor-based<br>detection tool in indoor fire scenarios. Abdelhamid et al. (2025) and Tateoka and Kidera<br>(2026) demonstrate that FMCW and mmWave radar can detect and localize stationary|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>36|



|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>37<br>victims in collapsed and smoke-filled environments through non-metallic materials, while<br>Zhang et al. (2024) extends this evidence to mining accident scenarios with conditions<br>analogous to Philippine typhoon disaster contexts. The review by Uzunidis et al. (2023)<br>further validates the hardware selection rationale, confirming that 60 GHz FMCW radar<br>achieves an appropriate balance between penetration depth and detection sensitivity for the<br>lightweight debris environments characteristic of the Philippine SAR context. Collectively,<br>these studies establish that the problem Sarwave addresses is real, that the sensing modality<br>chosen is appropriate, and that no existing system has yet been designed and validated<br>specifically for the Philippine disaster environment. Furthermore, the operational context<br>established across SAR literature, where victims range from fully mobile and disoriented<br>to stationary and minimally moving, directly informs the Sarwave system's target<br>population design, which is optimized for stationary conscious victims as the primary and<br>most challenging detection case, with mobile victims expected to fall within the classifier's<br>positive detection boundary as a natural consequence of their stronger Doppler signatures<br>rather than as an independently validated capability. The scan-and-hold operational<br>protocol adopted by Sarwave is consistent with systematic indoor search methodology<br>described across SAR literature, where methodical room-by-room scanning is standard<br>practice in smoke-obscured environments.<br>The through-wall and debris penetration literature defines both the capabilities and<br>the scope boundaries of the proposed system. Uzunidis et al. (2024) and Yilmaz et al.<br>(2024) establish that radar-based detection remains feasible through lightweight|



|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>38<br>nonmetallic debris but degrades significantly through dense materials such as reinforced<br>concrete above 20 cm thickness. Niyaz et al. (2024) and Pramanik and Islam (2024) further<br>confirm that single-sensor configurations are sufficient for detection tasks in these material<br>conditions, validating the Sarwave system's single-unit IWR6843AOP architecture. These<br>findings directly informed the decision to scope the system's testing to non-metallic<br>lightweight debris and to explicitly exclude reinforced concrete penetration from the<br>system's operational claims.<br>The signal processing literature informs the pre-classification data pipeline of the<br>proposed system. Xie et al. (2026), Zheng et al. (2025), Yin et al. (2024), and Liu et al.<br>(2023) collectively demonstrate that FMCW radar signals in cluttered environments require<br>dedicated noise suppression, clutter removal, and adaptive decomposition before reliable<br>features can be extracted. While the Sarwave system operates on processed point cloud data<br>rather than raw I/Q signals, bypassing the most computationally intensive signal processing<br>stages, these studies inform the feature engineering choices made at the point cloud level,<br>particularly the selection of Doppler velocity and spatial spread features that are robust to<br>environmental noise.<br>The AI and machine learning literature provides the strongest direct justification for<br>the SVM methodology adopted in this study. Nocera et al. (2024) identifies SVM as among<br>the most computationally efficient classifiers for radar-based sensing with accuracy<br>competitive with deep learning, while Ali et al. (2024) confirms that SVM is a validated<br>and appropriate classifier specifically for mmWave radar point cloud data. Ahmed and Cho|



**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** (2023) contextualize this choice within the broader healthcare radar ML landscape, noting that the lack of standardized benchmark datasets has limited cross-study generalizability, a gap that Sarwave partially addresses by combining publicly available datasets with locally collected data. Fusco et al. (2024) further informs the SVM training strategy by demonstrating the importance of including negative class examples, noise and clutter returns, to reduce false positives, a critical requirement for a rescue tool where misdirected search effort has direct operational consequences.  The domain gap between RadIOCD's wearable-radar mobile-subject collection methodology and the Sarwave fixed-radar stationary-target deployment context is addressed through an asymmetric quality-filtered dataset assignment strategy informed by the convergent evidence across the reviewed literature. Nocera et al. (2024) and Ali et al. (2024) establish that SVM performance on radar data is highly sensitive to feature distribution consistency between training and deployment conditions, a finding that directly motivates restricting RadIOCD's contribution to the human present class to near-stationary frames only, filtered by Doppler velocity ceiling, point density range, cluster coherence, and minimum SNR criteria. This quality filtering contains the domain gap effect to supplementary subject diversity enrichment rather than allowing it to contaminate the primary positive classification boundary. Ahmed and Cho (2023) further identify limited subject diversity as a primary source of generalization failure in healthcare radar ML systems, providing the specific justification for including RadIOCD's quality-filtered human frames as a supplementary source: expanding physiological diversity beyond the three local subjects without 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

39 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>40<br>compromising the hardware-matched dominance of local data in the human present class.<br>The asymmetric strategy, local data primary for both classes, RadIOCD supplementary<br>with class-specific filtering, is thus grounded in the convergent evidence of the reviewed<br>AI and machine learning literature rather than representing an ad hoc data management<br>decision.<br>The system deployment literature bridges the gap between laboratory research and<br>field-deployable prototypes. Srihari et al. (2022) demonstrates that complete FMCW radar<br>signal processing pipelines can be implemented on embedded hardware comparable to the<br>Raspberry Pi 5, while Chang et al. (2024) confirms that handheld mmWave radar form<br>factors are operationally viable for non-specialist rescue personnel. Together, these<br>deployment references validate that the proposed Sarwave system architecture, a single<br>IWR6843AOP sensor processed by a Raspberry Pi 5 with LCD output, is technically<br>feasible, practically deployable, and academically grounded in existing prototype<br>implementations.<br>Taken together, the reviewed literature reveals a clear and addressable gap: while<br>mmWave radar technology for human presence detection has been validated across<br>multiple application domains, no existing system integrates SVM-based point cloud<br>classification, approximate centroid localization, and embedded edge deployment into a<br>single compact device explicitly designed and evaluated for indoor fire search and rescue<br>in a Philippine disaster context. The Sarwave system is positioned to fill this gap, drawing<br>on the convergent evidence base established across all six reviewed thematic categories.|



### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

### **Definition of Terms** 

The following terms are defined as they are used within the context of this study: 

**Binary Classification.** A machine learning task in which the model assigns an input sample to one of exactly two categories. In this study, binary classification refers to the SVM model's task of determining whether a human target is present or absent within the radar's field of view based on extracted point cloud features. 

**Centroid.** The geometric mean position of a cluster of points in three-dimensional space. In this study, the centroid of a detected point cloud cluster is used as the approximate positional estimate of a human target and is displayed on the onboard LCD as a localization output. 

**Chirp.** A radar signal whose frequency increases linearly over a defined time interval, also referred to as a frequency sweep. In FMCW radar, a sequence of chirps is transmitted and mixed with reflected returns to extract range and velocity information from the environment. 

**Clutter.** Unwanted radar reflections from stationary or slowly moving objects in the environment, such as walls, furniture, and structural debris, that are not the intended 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

41 

**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** detection target. Clutter suppression is a critical step in radar-based human presence detection to prevent false positive classifications. **Doppler Velocity.** The radial velocity of a detected object relative to the radar sensor, derived from the frequency shift between the transmitted and received signals caused by target motion. In this study, Doppler velocity values extracted from point cloud data serve as discriminative features for distinguishing human micro-motion from stationary background clutter. **Edge Computing.** A computing paradigm in which data processing and inference are performed locally on a device near the data source, without dependence on a remote server or cloud platform. In this study, the Raspberry Pi 5 serves as the edge computing unit, enabling real-time SVM inference and LCD output without requiring network connectivity or external computing infrastructure. 

**False Negative.** A classification outcome in which the model predicts that no human target is present when a human target is actually present. In the context of search and rescue operations, a false negative is a critical failure mode as it may cause rescue personnel to bypass a location where a victim requires immediate assistance. **FMCW Radar (Frequency-Modulated Continuous Wave Radar).** A type of radar system that continuously transmits a signal whose frequency is modulated linearly 

|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE|
|---|



42 

**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** over time. By mixing the transmitted signal with its reflected return, the system extracts range and velocity information simultaneously. In this study, the IWR6843AOP operates as an FMCW radar sensor at 60 GHz. **IWR6843AOP.** The Texas Instruments IWR6843AOP is a single-chip 60 GHz mmWave radar sensor used in this study as the primary sensing hardware. The sensor integrates the transmit and receive antenna arrays, radio-frequency front end, analog-todigital converters, and a digital signal processor within a single compact package, and outputs processed point cloud data via USB interface. **Kernel Function.** A mathematical function used in Support Vector Machine classifiers to transform input feature data into a higher-dimensional space where linear class separation becomes feasible. In this study, the Radial Basis Function (RBF) kernel is the primary candidate for the SVM model given its general-purpose effectiveness on nonlinearly separable radar feature data. **Localization.** The process of estimating the position of a detected target within a defined coordinate frame. In this study, localization refers to the centroid-based approximate position estimation of a human target within the IWR6843AOP's single field of view, intended to provide first responders with a general directional indication of victim location rather than a precise coordinate fix. 

|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE|
|---|



43 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

**Micro-Doppler Signature.** The Doppler frequency components are produced by small, repetitive motions of a target, such as breathing, involuntary muscle tremors, and minor postural shifts. Micro-Doppler signatures enable the radar system to detect the presence of a stationary or minimally moving human target even when the person cannot actively signal their location. 

**mmWave Radar (Millimeter-Wave Radar).** A class of radar systems operating in the frequency range of 30 GHz to 300 GHz, corresponding to wavelengths between 1 mm and 10 mm. mmWave radar is unaffected by optical obstructions including smoke, dust, and darkness, making it suitable for deployment in hazardous indoor environments where vision-based sensors are non-functional. **Point Cloud.** A set of data points in three-dimensional space, each representing a location at which the radar detected a reflected signal above the noise threshold. In this study, the IWR6843AOP outputs processed point clouds via USB, with each point carrying spatial coordinates (x, y, z), radial velocity, and signal-to-noise ratio attributes that serve as inputs to the SVM classifier. **Point Density.** The number of radar point returns detected within a defined spatial region during a single measurement frame. In this study, point density serves as a primary 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

44 

**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** feature distinguishing human target, which produce dense, localized point clusters, from sparse background clutter returns. **Raspberry Pi 5.** A single-board computer used in this study as the onboard edge processing unit. The Raspberry Pi 5 receives point cloud data from the IWR6843AOP via USB, performs feature extraction and SVM inference in real time, and drives an LCD display with detection and localization output, all without requiring external computing devices or network connectivity. **Sarwave.** The name of the prototype system developed in this study, combining 'SAR' (Search and Rescue) and 'wave' (mmWave radar). Sarwave is an SVM-based human presence detection and localization system using a 60 GHz FMCW mmWave radar sensor, designed for deployment by first responders in indoor fire search and rescue operations where visual detection is impossible. **Search and Rescue (SAR).** Organized operations conducted to locate, access, stabilize, and transport victims of disasters including structural fires, building collapses, earthquakes, and typhoons. In this study, SAR refers specifically to indoor fire search and rescue scenarios where smoke, darkness, and structural hazards prevent visual detection of victims by first responders. 

|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE|
|---|



45 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

**Spatial Spread.** A measure of the geometric extent of a detected point cloud cluster, quantifying how broadly the radar returns are distributed in three-dimensional space. In this study, spatial spread serves as a discriminative feature that differentiates the compact reflection profile of a human body from the more diffuse or irregular returns of structural debris and environmental clutter. **Support Vector Machine (SVM)** **_._** A supervised machine learning algorithm that performs classification by identifying the optimal hyperplane that maximally separates data points belonging to different classes in a high-dimensional feature space. In this study, SVM is used for binary classification of human presence versus absence based on features extracted from mmWave radar point cloud data, selected for its computational efficiency and suitability for resource-constrained embedded deployment on the Raspberry Pi 5. 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

46 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

### **CHAPTER III: METHODOLOGY** 

### **Research Design** 

This study follows an experimental developmental research design based on the framework of Richey and Klein (2005), involving the development, training, deployment, and evaluation of the Sarwave prototype. Radar point cloud data for Support Vector Machine (SVM) training are collected under controlled indoor conditions to isolate human motion signatures and reduce environmental noise. Simulated rescue factors such as smoke, debris, reduced visibility, and varying target distances are introduced only during evaluation to assess system robustness and operational feasibility. This separation enables controlled feature learning while supporting objective assessment of system generalization under rescue-like conditions. The experimental approach allows systematic manipulation of variables to evaluate system robustness (Bounmy & Sisavath, 2023), while iterative refinement of radar integration, feature extraction, clutter suppression, and SVM tuning is performed in a safe and replicable simulated environment. 

### **Research Locale** 

The study will be conducted at the Lyceum of the Philippines University – Cavite in General Trias, Cavite. Prototype development and software deployment will take place within the Engineering Laboratories. High-smoke and physical obstruction trials will be executed in a designated, well-ventilated on-site enclosure to safely simulate fire conditions. This setup 

### COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

47 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

ensures the safe containment of non-toxic simulated smoke and structural debris without disrupting campus operations. 

### **Participants of the Study** 

Participants are divided into two categories: human bio-signatures representing the detection targets, and inanimate environmental variables representing clutter. Human targets will consist of healthy adult volunteers who will simulate trapped fire victims by remaining stationary in sitting, standing, or prone positions inside the testing grid while maintaining normal breathing patterns. Individuals with pre-existing respiratory conditions, such as asthma, will be excluded from any smoke trials. All participants will undergo an informed consent process and sign consent forms prior to data collection. 

### **Sampling Techniques** 

This study employs a non-probability, two-tiered sampling strategy combining purposive and quota sampling. Purposive sampling is applied to participant selection, targeting volunteers capable of reliably maintaining stationary postures to isolate radar sensitivity to breathing-induced micro-movements, and to material selection, deliberately choosing plywood sheets and standard drywall panels representative of common local residential and commercial debris. Quota sampling is then applied to maintain a balanced dataset across the nine-scenario test matrix, which crosses three distance zones: Zone A (0– 2 m), Zone B (2–4 m), and Zone C (4–6 m), with three obstruction conditions: open, partial 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

48 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

debris, and smoke-filled. A fixed quota of 100 frames per scenario (50 human-present, 50 human-absent) is maintained to ensure uniform representation across all testing environments. 

### **Research Instrument** 

|The sole instrument employed is a researcher-constructed Technical Validation|
|---|
|Matrix grounded in ISO/IEC 25010 and ISO/IEC 18305, comprising four sub-instruments:|
|a Functional Correctness Validation Matrix that logs confusion matrix outcomes and|
|computes Accuracy, FPR, FNR, and F1 Score for both the SVM-based and rule-based|
|approaches; a Reliability Consistency Matrix that quantifies intra-scenario stability through|
|standard deviation of Accuracy and FNR across repeated sessions per scenario, with an|
|acceptance criterion of SD ≤ ±0.05, and inter-scenario reliability by assessing whether|
|performance degrades gracefully across varying obstruction conditions; a Performance|
|Efficiency Logging Tool that captures per-frame processing latency and resource utilization|
|via timestamps and system monitors embedded natively in the Python pipeline; and a|
|Localization Acceptability Matrix that computes Euclidean distance error between|
|DBSCAN centroid estimates and laser-measured ground truth coordinates on true positive|
|frames only, with a practical acceptance threshold of ≤ 1.0 m mean aggregate error (Bannur|
|et al., 2026; Uzunidis et al., 2024).|



COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

49 

**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE Data Gathering Procedure** The Sarwave system uses a two-source dataset strategy combining a publicly available dataset with prototype-collected data, following an asymmetric source assignment that reflects the different domain characteristics of each source. **RadIOCD (2024).** serves a supplementary role in both classes. For Class 1 (human present), quality-filtered RadIOCD human frames meeting four criteria, Doppler velocity below 0.5 m/s, point density within the range observed in local Class 1 data, single dominant DBSCAN cluster coherence, and minimum SNR threshold, are included as a minority source to expand subject physiological diversity beyond the three locally collected subjects. For Class 0 (human absent), subsampled RadIOCD non-human object returns provide supplementary indoor clutter variety. In both roles RadIOCD is explicitly minorityweighted relative to local data. The domain gap arising from RadIOCD's wearable radar configuration, logistics hall environment, and IWR1443BOOST sensor, distinct from the prototype's handheld IWR6843AOPEVM, is acknowledged and managed through the quality filtering protocol for Class 1 and stratified subsampling for Class 0. RadIOCD contains 76,821 labeled samples and is publicly available via Zenodo. **Prototype-collected Data.** This constitutes the dominant primary source for both classes, collected using the IWR6843AOPEVM in a controlled laboratory environment. Class 1 data grounds the positive classification boundary, the most safety-critical component where false negatives carry the highest operational cost, in hardware-matched, 

|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE|
|---|



50 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

environment matched, behavior-matched recordings. Class 0 data capture empty room and background clutter returns from the same prototype environment. The combined dataset is partitioned at the session level to prevent temporal data leakage from high frame-to-frame correlation in consecutive radar recordings. All frames from a given session are assigned exclusively to one subset, training, validation, or test, with no session contributing to more than one subset. The dataset is split into approximately 70% training, 15% validation, and 15% test, with stratified allocation preserving class balance across all subsets. 

### **Data Collection Protocol** 

Each session follows a standardized protocol. The radar unit is mounted at 1.0–1.2 meters above the floor, simulating handheld scan-and-hold operation, and remains stationary throughout. The operator observes a minimum 2-second stabilization pause before each recording window to eliminate residual hand-motion Doppler contamination. The test space is divided into three distance zones: Zone A (0–2 m), Zone B (2–4 m), and Zone C (4–6 m). All sessions are conducted under open, unobstructed conditions with no environmental factors introduced, ensuring the training dataset reflects clean baseline point cloud characteristics without confounds. A minimum of 50 frames per class per zone are recorded across two behavioral states: 

### COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

51 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|**Class 1 (Human Present).**Stationary state: subject stands still with only natural<br>breathing and involuntary micro-motion, representing the worst-case scenario of a trapped<br>or incapacitated victim. Slow-moving state: subject makes minor positional adjustments<br>consistent with a conscious but disoriented victim. Both states are labeled Class 1 as they<br>represent the full operational spectrum of victim behavior.<br>**Class 0 (Human Absent).**Subject exits the detection zone entirely. Recordings<br>capture background clutter returns from the empty environment. Multiple subjects are<br>utilized across sessions to capture inter-subject physiological variability. Each 60-frame<br>session is assigned as a unit during splitting, ensuring no sessionlevel data leakage.|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>52<br>Environmental factors, smoke and thin non-metallic material obstruction, are<br>introduced exclusively during testing and evaluation, not training. This separation is<br>intentional: mmWave radar at 60–64 GHz is physically predicted and experimentally<br>established to be unaffected by these conditions, so the trained classifier is expected to<br>generalize without environment-specific retraining. At 60–64 GHz, the 5mm wavelength<br>is orders of magnitude larger than smoke particle diameters, producing negligible scattering<br>and zero signal attenuation (Chen et al., 2024; Lu et al., 2020). For thin non-metallic<br>materials, Hosseini et al. (2021) quantified penetration loss of only 2–9 dB for wood and<br>glass at mmWave frequencies, well within the IWR6843AOPEVM's operational SNR<br>margin.|



|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|**Feature Extraction**<br>Following data collection, four discriminative features are extracted from each<br>labeled point cloud frame to form the input representation for the SVM classifier.<br>Point density, the total number of points detected within the frame above the CFAR noise<br>threshold, serves as the primary discriminative feature. Human targets produce dense,<br>localized clusters of returns due to the reflective surface area of the body, whereas empty<br>rooms and debris produce sparse or diffuse returns. Doppler velocity spread, computed as<br>the standard deviation of Doppler velocity values across all points in the detected cluster,<br>captures human micro-motion signatures, involuntary muscle tremors, slight postural<br>shifts, and breathing-related chest displacement, which produce a characteristic spread of<br>low-magnitude Doppler values distinct from the near-zero velocity of static clutter. Spatial<br>spread, the standard deviation of x, y, and z coordinates across all detected points, quantifies<br>the geometric extent of the cluster. A human body produces a compact, consistent cluster<br>profile distinct from the diffuse or irregular spread of structural debris reflections. Centroid<br>position, computed as the arithmetic mean of all point coordinates within the cluster, is<br>included primarily for localization output but also captures range-dependent signal<br>characteristics through the centroid's distance from the radar origin.<br>All four features are assembled into a normalized feature vector per frame. Min-max<br>normalization is applied to bring all features onto a common scale [0, 1], ensuring no single<br>feature dominates the SVM decision boundary due to magnitude differences.|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>53|



|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**<br>**Training Strategy**<br>A binary SVM classifier is implemented using scikit-learn and deployed on the<br>Raspberry Pi 5, with the RBF kernel selected for its established effectiveness on non-<br>linearly separable radar feature data (Ali et al., 2024; Nocera et al., 2024). Training<br>proceeds in two stages.<br>Stage 1 trains locally collected data only, establishing a hardware-matched baseline<br>independent of any public dataset contribution. Stage 2 retrains the complete asymmetric<br>combined dataset, local data as the dominant primary source for both classes, supplemented<br>by quality-filtered RadIOCD frames. The performance delta between Stage 1 and Stage 2<br>quantifies the specific contribution of RadIOCD supplementation. The Stage 2 model<br>constitutes the final production classifier.|
|---|
|**Cross-Validation and Hyperparameter Tuning**<br>Five-fold stratified cross-validation is applied throughout, with each fold preserving<br>class balance. Hyperparameter optimization is performed via grid search over C  {0.1, 1,<br>10, 100} and γ  {0.001, 0.01, 0.1, 1, 'scale'}, selecting the combination yielding the highest<br>mean cross-validation F1 score (Ham et al., 2025). Accuracy, precision, recall, FPR, FNR,<br>and F1-score are recorded per fold with mean and standard deviation reported. Accuracy<br>alone is insufficient for imbalanced binary classification as it reflects majority class<br>performance rather than minority class detection capability (Chicco and Jurman, 2020). F1-|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>54|



**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** score is the primary optimization criterion as it balances precision and recall without assuming equal error costs (Sokolova and Lapalme, 2009). FNR is the primary safety metric quantifying missed victim rate; FPR is the secondary metric quantifying false alarm rate. 

### **Project/System Construction Procedure** 

The Sarwave system was developed following the Iterative Software Development Life Cycle (SDLC) model, which begins with a simple implementation of a small set of requirements and iteratively enhances evolving versions until the complete system is ready for deployment (Ndrecaj et al., 2023). This model was selected over waterfall or sequential alternatives because embedded systems involve intricate interactions between hardware and software components that cannot be fully specified in advance and must be validated through physical testing, requiring parallel hardware and software development tracks that are refined through successive iterations. The iterative approach is further justified by the ML component of the system, as ML-based IoT application development follows an iterative roadmap where audited issues from each cycle guide corrections in the next, consistent with Sarwave's two-stage SVM training and deployment validation procedure. 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

55 



<!-- Start of picture text -->
| Phase 1 -- Requirement and System Design |<br>| Phase 2 — Hardware Integration and Validation |<br>a<br>Phase 3 -- Software Pipeline Development |<br>Refrain (<br>| Phase 4 — Data Collection and Model Training |<br>ae Phase 5 — System Evaluation |<br><!-- End of picture text -->

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

complete real-time processing pipeline in Python 3.11 on the Raspberry Pi 5, with each pipeline stage validated independently before end-to-end integration testing and latency profiling against the target frame period. The fourth phase conducted prototype data collection, two-stage SVM training with five-fold stratified cross-validation and grid search hyperparameter optimization, and deployment validation confirming 100% classification consistency between the development machine and deployed Raspberry Pi 5. The fifth phase conducted formal evaluation across all nine test scenarios, compared SVM performance against the rule-based baseline using McNemar's test, evaluated localization accuracy on true positive frames, and assessed results against ISO/IEC 25010:2023 quality characteristics and IEC 62676-4 benchmarks. 

### **System Overview** 

|**a.) Input Domain.**The physical setting is an indoor fire scene characterized by dense|
|---|
|smoke, structural debris, and zero-light conditions, which render traditional cameras<br>unusable. For the physical hardware, the TI IWR6843AOPEVM mmWave radar sensor|
|acts as the primary hardware input. Operating in the 60 GHz frequency band, it emits|
|Frequency-Modulated Continuous-Wave (FMCW) signals that penetrate smoke and|
|debris to capture raw reflections off physical structures and living human targets.|
|Furthermore, the digital data source combines publicly available mmWave radar point|
|cloud repositories with customized datasets collected by the researchers in a controlled|
|laboratory environment, serving as the baseline for model training.|



|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE|
|---|



57 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

**b.) Process Domain.** The processing phase is executed entirely on the onboard Raspberry Pi 5 single-board computer, which handles raw data manipulation, feature extraction, and machine learning inference locally. The incoming raw 3D point cloud stream undergoes clutter removal and noise filtering to isolate human targets. This step removes static reflections from obstacles (walls, furniture) and filters out signal scattering caused by moving smoke and heat currents. The system extracts spatial and kinematic features from the filtered point cloud, including point density, centroid position, Doppler velocity, and spatial spread. These engineered features are fed into a locally deployed Support Vector Machine (SVM) model. **c.) Output Domain.** The output domain translates high-dimensional processed data into actionable, visual feedback on the device's Onboard LCD Display. The display renders two lines of text output: the detection result (HUMAN DETECTED / NOT DETECTED) and the estimated position (e.g., RANGE: 2.4 m | DIR: 15° LEFT), providing the responding firefighter with approximate distance and direction without requiring graphical rendering. 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

58 



<!-- Start of picture text -->
60 GHz FMCW chirps:<br>environment<br>pes ><br>(60 IWR6843AOPEVM :sien LCD display<br> Ghz FMWC radar point USB DBSCAN - SVM (Detection - Distance &<br>cloud via USB) } Localization - LCD driver a }<br>:<br>A<br>5V power<br>“5ee<br>Portable power supply<br>(power bank)<br>X Y<br><!-- End of picture text -->

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

### **_Prototype Enclosure Design_** 

The complete assembly is housed in a compact enclosure appropriate for hand-held one person operation by a firefighter or first responder. Generally, this will include a handle at the top to be carried in one arm when necessary, and an mmWave radar sensor at the front, and an LCD display. 

### **_Radar Configuration_** 

The IWR6843AOPEVM is configured using TI's mmWave SDK and the chirp configuration profile (.cfg file). The chirp parameters are set to optimize detection sensitivity for short-range indoor human presence scenarios (0–6 meters). Key configuration parameters include start frequency (60 GHz), frequency slope, idle time, ADC samples per chirp, number of chirps per frame, and frame periodicity. Static clutter removal is enabled in the DSP configuration to suppress zero-velocity returns from walls and furniture, while sensitivity is tuned to retain residual micro-motion returns from a minimally moving human target. The configured output is a structured 3D point cloud streamed to the Raspberry Pi 5 via USB serial at a defined frame rate. 

### **_Real-Time Pipeline_** 

The complete pipeline runs on the Raspberry Pi 5 in real time. Per frame, it executes sequentially: two-stage static clutter removal combining hardware-level DSP suppression from the IWR6843AOP and a software Doppler velocity threshold filter; DBSCAN 

### COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

60 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

clustering to group surviving points into distinct clusters without requiring a pre-specified cluster count; feature extraction computing point density, Doppler velocity spread, spatial spread, and centroid position from the largest cluster, normalized using scaler parameters saved during training; and SVM inference producing a binary output. When Class 1 is returned, centroid-based localization computes range in meters and azimuth angle in degrees, displayed on the I2C LCD in real time. End-to-end latency per frame is measured against a target of less than one radar frame period (50–100 ms). 

### **Multiple Constraints Analysis** 

To ensure that the Sarwave system is evaluated as a holistic engineering solution, the following multiple constraint analysis and strategic project plans have been developed: 

### **Table 1** 

### _Analytical Hierarchy Evaluation Rubric_ 

|**Score**|**5**|**4**|**3**|**2**|**1**|
|---|---|---|---|---|---|
|Classifica<br>tion|Excellent|Good|Moderate|Poor|Critical|
|Sustainab|<8W; >6-hr|8–12W; 4–|13–16W;|17–20W;|>20W;|
|ility|runtime;<br>recyclable<br>shell|6 hr<br>runtime;<br>repairable<br>shell|3–4 hr<br>runtime;<br>custom<br>modules.|2.5–3.5 hr<br>runtime;<br>proprietary<br>parts.|<2.5-hr<br>runtime;<br>high e-<br>waste<br>footprint.|



COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

61 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

|Economic|<₱25,000;<br>locally<br>sourced|₱25,000–<br>₱35,000;<br>stable<br>sourcing|₱35,001–<br>₱45,000;<br>long import<br>lead times|₱45,001–<br>₱55,000;<br>high<br>sourcing<br>risk|>₱55,000;<br>financially<br>prohibitive.|
|---|---|---|---|---|---|
|Social|Native 3D<br>localization;<br><1kg|Native 3D<br>localization;<br>1–1.5kg|2D/1D<br>detection;<br>manageable<br>weight.|1D<br>detection<br>only;<br>heavy/bulky|No<br>directional<br>capability;<br>bad<br>ergonomics|
|Health<br>and<br>Safety|No thermal<br>hazards;<br>enclosed<br>fail-safes|Manageable<br>heat; stable<br>power<br>delivery|Noticeable<br>heat under<br>load;<br>requires<br>minor<br>handling.|High heat<br>generation;<br>casing<br>overheating<br>risks|High risk of<br>battery<br>thermal<br>runaway;<br>exposed<br>wiring or<br>sharp edges|
|Environm<br>ental|Fully<br>recyclable;<br>minimal e-<br>waste.|Mostly<br>recyclable;<br>low<br>hazardous<br>content;|Mixed<br>recyclabilit<br>y; some<br>components<br>complicate<br>disposal.|High e-<br>waste risk;<br>difficult<br>local<br>disposal.|Non-<br>recyclable<br>proprietary<br>parts;<br>severe e-<br>waste<br>hazard.|



### **_Plan A_** 

The Sarwave system integrates an Infineon BGT60TR13C radar with an NVIDIA Jetson Orin Nano to deliver high-precision life detection and 3D localization. Utilizing 60 GHz FMCW technology, the device achieves a 0.05m range resolution, allowing it to 

### COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

62 



<!-- Start of picture text -->
=@<br><!-- End of picture text -->



<!-- Start of picture text -->
Bx<br><!-- End of picture text -->

## =@ Bx 2 = 



<!-- Start of picture text -->
=<br><!-- End of picture text -->



<!-- Start of picture text -->
2<br><!-- End of picture text -->



<!-- Start of picture text -->
TT<br>7<br>1|<br>ia<br>a TT<br><!-- End of picture text -->





<!-- Start of picture text -->
SARWAVE e<br>94<br>s °<br>3B<br>e e<br><!-- End of picture text -->

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

is not feasible; the entire unit must be replaced upon failure, generating significant electronic waste and increasing long-term operational costs. 

**Economic (Score: 1).** Local sourcing is a major risk as most components, particularly the Jetson Orin Nano and the Infineon BGT60TR13C, are not stocked in the Philippines. International procurement adds 2–4 weeks in lead time and import duties, threatening thesis timelines. For BFP deployment, the PHP 60,000+ per-unit cost makes large-scale adoption economically prohibitive under the constrained budgets of RA 9514. 

**Table 2** 

### _Scoresheet of Plan A_ 

||**Radar Sensor**|**Processing**<br>**Unit**|**Power System**|**Enclosure**|
|---|---|---|---|---|
|**Item**|Infineon<br>BGT60TR13<br>C|NVIDIA Jetson<br>Orin Nano|Li-ion 6S2P<br>pack|6061<br>aluminum|
|**Feature**|57–64 GHz<br>FMCW|6-core Arm<br>A78AE|60Wh<br>mAh×2)|Integrated heat<br>spreader|
|**Price**|PHP 21,733|PHP 35,000 –<br>40,000|PHP 3,000 –<br>4,000|PHP 1,000 –<br>2,000|



COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

65 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**<br>**Social (Score: 3).**Ergonomically, the weight of the aluminum enclosure reduces<br>maneuverability in debris-filled environments, increasing operator fatigue during extended<br>search operations. While the system delivers 3D spatial data, the physical burden partially<br>offsets that advantage. Socially, the high cost remains a significant barrier to access for<br>most municipal BFP stations without external or supplemental funding.|
|---|
|**Health and Safety (Score: 2).**The Jetson Orin Nano generates significant heat<br>under sustained load, and the custom 6S2P Li-ion battery pack carries an elevated thermal<br>runaway risk compared to commercial off-the-shelf alternatives. The 6061-aluminum<br>frame, while structurally rigid, acts as a thermal conductor and may become a burn hazard<br>when the device is operated in high-ambient-temperature environments such as active<br>firegrounds.|
|**Environmental (Score: 1).**The 6061-aluminum enclosure and proprietary Jetson<br>module are not easily recyclable at end-of-life, particularly within local Philippine disposal<br>infrastructure. The custom 6S2P Li-ion battery pack poses hazardous waste risks upon<br>failure or disposal. The high replacement rate, driven by non-repairable components,<br>significantly amplifies the system's cumulative e-waste footprint over its operational<br>lifespan.<br>**_Plan B_**<br>The system centers on the Acconeer A121, a pulsed coherent radar known for its<br>extremely low power consumption and high precision in detecting micro-motions|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>66|











# = 



<!-- Start of picture text -->
q LL|<br>}<br>|<br>|<br>|<br>}<br>41| |<br>———"<br><!-- End of picture text -->



<!-- Start of picture text -->
|<br>i}<br><!-- End of picture text -->



<!-- Start of picture text -->
SARWAVE - top-down view - IWR6843A0OP 60 GHz tect<br>89<br>+=<br>5<br>\ 3<br>ea~eeeS 1.33m<br>5.82m<br>. \ +> 0.039 m/s<br>20.4 dB<br>a 7318<br>o,*'<br>"<br><!-- End of picture text -->

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

recyclable, and all core components are commercially available with no proprietary lockin, making the system highly maintainable across its service life. 

**Economic (Score: 4).** At approximately ₱28,000 in total component cost, Plan B is 

the most economical architecture. Key parts, including the Orange Pi 5 and the power bank, are readily available through local distributors and online platforms such as Shopee, eliminating import delays. The low unit cost makes it the most viable candidate for scaled deployment across multiple BFP stations without requiring external funding. 

### **Table 3** 

### _Scoresheet of Plan B_ 

||**Radar Sensor**|**Processing**<br>**Unit**|**Power System**|**Enclosure**|
|---|---|---|---|---|
|**Item**|Acconeer A121<br>(XM125 kit)|Orange Pi 5<br>(8GB)|USB-C PD 65<br>W power bank|PETG 3D-<br>printed shell|
|**Feature**|Pulsed coherent<br>radar|8-core|111 Wh|Stable below<br>60°C|
|**Price**|PHP 7,447|PHP 17,000-<br>22,000|PHP 3,2000 –<br>4,000|PHP 500 –<br>1,000|



COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

69 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

**Social (Score: 3).** The Acconeer A121 outputs a one-dimensional range profile, which means it can detect the presence of a victim within a line-of-sight cone but cannot determine azimuth or elevation. In practice, rescue personnel must manually pan and tilt the device to triangulate a victim's position, extending the time they spend in smoke-filled or structurally compromised environments. This limitation directly reduces the operational utility of the device in time-critical scenarios. **Health and Safety (Score: 4).** Internal heat generation is negligible due to the lowpower configuration, ensuring that the PETG shell remains structurally stable and safe to handle throughout an operation. The use of a commercially tested power bank with integrated battery management eliminates the thermal runaway risk associated with custom battery assemblies. The lightweight form factor also reduces ergonomic strain, supporting safer, longer-duration deployments. **Environmental (Score: 5).** Plan B carries the lowest environmental footprint of all three architectures. PETG is a recyclable thermoplastic fully compatible with standard plastic recycling streams, and the 3D-printed enclosure can be reprinted locally, eliminating international shipping emissions. All components are commercially standardized, reducing e-waste through reuse and component-level replaceability. The low energy draw further minimizes the system's operational carbon footprint. 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

70 











<!-- Start of picture text -->
|<br>| q<br>a<br>|<br><!-- End of picture text -->





<!-- Start of picture text -->
©SARWAVE °<br>SVM<br>°<br>0.38 m<br><!-- End of picture text -->

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

replacing the entire chassis. While power consumption is moderately higher than Plan B, it remains within acceptable limits for the operational demands of a live rescue scenario. 

**Economic (Score: 4).** The total system cost falls within the ₱29,000–₱32,000 range, fitting comfortably within standard academic project budgets. The Raspberry Pi 5 and the commercial power bank are both readily available through local Philippine distributors, minimizing procurement lead times and import risk. The IWR6843AOPEVM requires international sourcing, but its stable supply chain and moderate cost do not pose a significant timeline risk. 

### **Table 4** 

### _Scoresheet of Plan C_ 

||**Radar**<br>**Sensor**|**Processing**<br>**Unit**|**Power**<br>**System**|**Enclosure**|
|---|---|---|---|---|
|**Item**|IWR6843AOPE<br>VM|Raspberry Pi 5<br>(8GB)|20,000 mAh<br>power bank|3D-printed ABS|
|**Feature**|60–64 GHz<br>FMCW|Cortex-A76|74 Wh|Stable below<br>85°C|
|**Price**|PHP 9,232|PHP 17,999 –<br>20,000|PHP 1,500 –<br>2,000|PHP 500 –<br>1,000|



COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

73 

**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE Social (Score: 5).** The TI IWR6843AOPEVM produces native 3D point clouds with simultaneous range, azimuth, and elevation output, enabling responders to immediately localize victims in three-dimensional space without manual scanning. The lightweight ABS enclosure keeps total system weight within an ergonomic range, reducing fatigue during extended operations. This combination of spatial awareness and physical usability directly maximizes the device's utility in active rescue scenarios. **Health and Safety (Score: 4).** The ABS enclosure is rated to withstand sustained temperatures up to 85°C, providing a reliable thermal barrier in fireground conditions. Power delivery through a commercially certified power bank eliminates the risks of custom battery packs, including thermal runaway and cell imbalance. Internal heat generation from the Raspberry Pi 5 and IWR6843 module is manageable and does not pose a burn hazard to the operator under normal operating conditions. **Environmental (Score: 4).** ABS is a widely recyclable thermoplastic compatible with local waste management streams, and the 3D-printed enclosure format allows panellevel repair, reducing full-unit disposal frequency. The commercial power bank is a standardized product with established recycling pathways. While the IWR6843AOPEVM requires international sourcing, its stable supply chain minimizes repeated procurement shipping. The overall environmental footprint is moderate and well-managed relative to the system's operational demands. 

|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE|
|---|



74 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

### **_Overall Comparison_** 

### **Table 5** 

### _Comparison of Scoresheets of Plan A, B, and C_ 

|**Constraint**|**Weight**|**Plan A**|**Plan B**|**Plan C**|
|---|---|---|---|---|
|**Sustainability**|15%|2|5|4|
|**Economic**|20%|1|4|4|
|**Social (Utility)**|20%|3|3|5|
|**Health and Safety**|25%|2|4|4|
|**Environmental**|20%|1|5|4|
|**Weighted Total Score**|**100%**|**1.80**|**4.15**|**4.20**|



To determine the most viable architecture, the constraints are weighted based on 

their critical importance to a functional rescue prototype. Weights: Sustainability (15%), Economic (20%), Social (20%), Health and Safety (25% is the highest weight, as responder safety is paramount), and Environmental (20%). The exact equation can be seen below. 

𝑇𝑜𝑡𝑎𝑙 𝑆𝑐𝑜𝑟𝑒= 0.15𝑆+ 0.20𝐸𝐶 + 0.20𝑆𝑂 + 0.25𝐻𝑆 + 0.20𝐸𝑛 **(1)** 

The quantitative hierarchy establishes Plan C (Total: 4.20) as the optimal architecture for the Sarwave system. Plan A (1.80) is disqualified by its severe economic barriers and hazardous thermal profile under heavy computational load. While Plan B (4.15) excels in sustainability and affordability, its 1D sensing limitation creates a dangerous bottleneck in social utility, forcing rescuers to manually scan rooms. Plan C prevails because it maximizes both Social Utility and Health & Safety. By providing native 3D 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

75 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

spatial data in a thermally resistant, lightweight ABS enclosure powered by a stable commercial power bank, it ensures rapid victim localization without putting the operator at risk. It fits within standard academic funding scopes while strictly adhering to the rigorous safety constraints required for fire service deployment. 

### **System Model / Project Model** 

The operational flow of the Sarwave system, as illustrated in Figure 5, follows a linear pipeline from raw signal acquisition to real-time status notification. This process integrates high-frequency radar sensing with machine learning inference to transform raw electromagnetic reflections into actionable search and rescue data. 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

76 



<!-- Start of picture text -->
FMCW<br>(WR6S430PEVM transmits 60 Hz chirps)<br>Reflected RF signal<br>Onboard DSP processing<br>(Range-Doppler, CFAR, angle estimation)<br>Ct— CT<br>H1<br>i '<br>' Point cloud preprocessing '<br>H (Static clutter removal, noise filtering) '<br>'i<br>i '<br>'Cleaned point cloud frame<br>'\<br>i<br>DESCAN clustering<br>H (Group points into distinct clusters)<br>i<br>i i<br>Labeled point clusters per franie<br>'i<br>i '<br>Feature extraction<br>' (Density, velocity spread, spatial spread) '<br>ii<br>' i<br>Normalized feature vector [F1, #2._fn]<br>i i<br>i<br>SVM Classifier<br>(Binary decision: human (1) / no human (0)) '<br>{ i<br>lass Isbe Cluster centroid '<br>; _. Centroid-based localization<br>Detection decision Estimate direction and distance<br>i i<br>Pee ee ee eee eee ee ee ee eee ee ee ee ee ee ee be eee ee ee ee ee ee eee eee ee eee eee ee ee eee!<br>Detection + ee output<br>LCD display output Online Dashboard<br>Onsite result - portable Offsite result - non-portable<br><!-- End of picture text -->

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

The sensor then streams a structured 3D point cloud, containing spatial coordinates (x, y, z), Doppler velocity, and signal intensity, to the Raspberry Pi 5 via USB. 

The Raspberry Pi performs clutter removal and noise filtering to isolate dynamic returns. The refined data is processed through DBSCAN clustering, which groups proximate points into distinct objects, separating potential targets from environmental noise. From these clusters, the system extracts feature vectors based on point density, velocity spread, and spatial distribution. Furthermore, feature vectors are fed into a Support Vector Machine (SVM) classifier for binary classification (Human Present vs. Absent). Upon a positive detection, the system calculates the survivor's centroid-based localization, including distance and angular direction. These results are updated in real time on the I2C LCD, providing first responders with immediate, onsite situational awareness. 

### **Sample Size Justification** 

The minimum operational evaluation target for this study is 100 frames per scenario (50 per class), derived from the standard proportion estimation formula for binary outcomes. The derivation and justification are presented below. The evaluation target was estimated using the standard binary proportion approximation with a specified margin of error at a given confidence level (Cochran, 1977): 



COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

78 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>79<br>Substituting the design parameters into Equation 2 yields the baseline sample size<br>requirements. A confidence level of 95% (z = 1.96) was selected in accordance with<br>foundational sampling theory (Cochran, 1977). The conservative assumed proportion was<br>set at p = 0.85, established as a minimum baseline performance benchmark for radar-based<br>classification systems in operational environments (Nocera et al., 2024). Finally, the target<br>margin of error was set at E = 0.10, representing a ±10 percentage point precision constraint.<br>This boundary is mathematically justified to counteract inherent cross-validation sampling<br>noise when establishing predictive model boundaries on localized datasets.<br>Substituting𝑧= 1.96(95% confidence level),𝑝= 0.85(target minimum accuracy<br>threshold), and𝐸= 0.10(±10percentage point margin of error) into the foundational<br>sampling framework (Cochran, 1977) yields𝑛≈49, which is rounded up to an operational<br>target of 50 frames per class. This structure produces 50 frames per class per scenario,<br>totaling 100 frames per scenario. At the aggregate level across all nine scenarios,<br>representing 900 total test frames (450 per class), confidence intervals narrow substantially,<br>providing robust statistical support for the system-level viability claim. Consequently, the<br>primary operational conclusion is drawn at this aggregate level. Because radar frames are<br>acquired from continuous temporal sequences, adjacent observations may exhibit partial<br>temporal correlation. The reported confidence interpretation should therefore be understood<br>as an operational evaluation estimate rather than a strict independent and identically<br>distributed (IID) statistical inference.|



### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

### **Test Scenarios and Procedure** 

Performance is evaluated across nine test scenarios defined by distance zone and 

environmental conditions. Each scenario evaluates a minimum of 50 held-out labeled frames, 25 human present, 25 human absent, as illustrated in Table 6 below. 

### **Table 6** 

### _Test Scenario Matrix_ 

|**Environment**|**Zone A**<br>**(0–2 m)**|**Zone B**<br>**(2–4 m)**|**Zone C**<br>**(4–6 m)**|
|---|---|---|---|
|**Open**|Scenario 1:|Scenario 2:|Scenario 3:|
||100 Frames|100 Frames|100 Frames|
||(50 per class)|(50 frames)|(50 frames)|
|**Partial Debris**|Scenario 4:|Scenario 5:|Scenario 6:|
||100 Frames|100 Frames|100 Frames|
||(50 per class)|(50 frames)|(50 frames)|
|**Smoke-filled**|Scenario 7:|Scenario 8:|Scenario 9:|
||100 Frames|100 Frames|100 Frames|
||(50 frames)|(50 frames)|(50 frames)|
|**Total across all**||||
|**scenarios**||||
||300 Frames|300 Frames|300 Frames|



Each scenario follows a standardized procedure. The environment is prepared per 

condition, debris arranged between subject and radar for debris scenarios, smoke 

introduced until optical visibility is eliminated for smoke scenarios. The system is powered 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

80 



<!-- Start of picture text -->
Zone A: 0-2m Zone B: 2-4 m Zone C: 4-6 m<br>.- -<br>OC<br>—_——<br>C) S = subject postion 2m 2m 2m<br><!-- End of picture text -->

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

### **Project Testing, Evaluation, and Validation** 

Prior to formal evaluation, a consistency check is performed using 100 held-out labeled feature vectors (50 per class) to validate cross-platform inference equivalence between the development environment and the Raspberry Pi 5 deployment, ensuring that differences in FPU precision, library-specific kernel implementations, or compiler optimizations do not introduce classification drift. System evaluation is then structured against four ISO/IEC 25010:2023 quality characteristics Safety (via FNR), Functional Correctness (via detection accuracy), Reliability (via metric standard deviation across repeated trials), and Performance Efficiency (via end-to-end processing latency) with detection probability and false alarm rate additionally benchmarked against IEC 62676-4 criteria. All performance metrics are derived from the binary classifier's confusion matrix on the held-out test set, with outcomes classified as TP, TN, FP, or FN corresponding to correct and incorrect detections of human presence (Class 1) and absence (Class 0). 

### **Table 7** 

### _Performance Metrics and SAR Relevance_ 

|**Metric**|**Definition**|**Relevance to SAR**<br>**Context**|
|---|---|---|
|Detection Accuracy|Equation 3|Overall correctness of|
|||classification|



COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

82 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|Precision<br>Equation 4<br>Proportion of detections<br>that are real humans; low<br>precision wastes rescue<br>effort|
|Recall (Sensitivity)<br>Equation 5<br>Proportion of actual<br>humans detected; low<br>recall risks missing<br>victims|
|False Positive Rate (FPR)<br>Equation 7<br>Rate of phantom<br>detections; high FPR<br>misdirects rescue<br>resources<br>False Negative Rate (FNR)<br>Equation 8<br>Rate of missed victims;<br>high<br>FNR is the critical failure<br>mode in SAR<br>F1-Score<br>Equation 10<br>Harmonic mean of<br>precision and recall;<br>primary model<br>Selection criterion|
|Reliability<br>Standard deviation of<br>Accuracy and FNR values<br>If a system is consistently<br>accurate|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>83<br>Localization Error (LE)<br>Euclidean distance between<br>Directional guidance|



### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

Performance Efficiency Time Behaviour Detection within a consistent and acceptable timeframe **Detection Accuracy.** It is defined as the proportion of correctly classified frames, both true positives and true negatives, out of the total number of evaluated frames across a given scenario or set of scenarios. It provides a general measure of overall classification correctness but must be interpreted alongside FNR and F1-score, as it can be misleading under class imbalance. 



**Precision** . This quantifies the proportion of positive detections that correspond to actual human presence. Precision serves as the core metric to determine class agreement between physical data tracking and the positive classifications output by the system. A low precision value indicates that the system produces frequent false alarms, flagging nonhuman objects, such as debris or structural clutter, as occupied. While this does not directly endanger victims, it misdirects rescue personnel and degrades operational trust in the system over repeated deployments. 



**Recall.** This quantifies the proportion of actual human presences that the system successfully detected. Recall is mathematically equivalent to the complement of the False 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

84 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

Negative Rate and is included here as a secondary expression of the same underlying failure mode, reported alongside FNR to maintain consistency with standard binary classification 

evaluation conventions. 

𝑇𝑃 𝑅𝑒𝑐𝑎𝑙𝑙= **(5)** 𝑇𝑃+ 𝐹𝑁 𝑅𝑒𝑐𝑎𝑙𝑙= 1 −𝐹𝑎𝑙𝑠𝑒 𝑁𝑒𝑔𝑎𝑡𝑖𝑣𝑒 𝑅𝑎𝑡𝑒 **(6)** 

**False Positive Rate.** This quantifies the proportion of human-absent frames that the system incorrectly classified as occupied. A high FPR indicates that the system generates phantom detections at a rate that would systematically misdirect rescue resources toward unoccupied areas, representing an operationally costly failure mode distinct from victim misses. 

𝐹𝑃 𝐹𝑎𝑙𝑠𝑒 𝑃𝑜𝑠𝑖𝑡𝑖𝑣𝑒 𝑅𝑎𝑡𝑒= **(7)** 𝐹𝑃+ 𝑇𝑁 

**False Negative Rate.** The False Negative Rate is the proportion of actual humanpresent frames incorrectly classified as human absent. In the SAR domain, a false negative constitutes the most critical system failure: a missed victim. Nocera et al. (2024) confirm that FNR is the primary safety metric for any life-sign detection system deployed in rescue operations. FNR is therefore weighted as the highest-priority performance criterion in all result interpretations. 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

85 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

𝐹𝑁 𝐹𝑎𝑙𝑠𝑒 𝑁𝑒𝑔𝑎𝑡𝑖𝑣𝑒 𝑅𝑎𝑡𝑒= **(8)** 𝐹𝑁+ 𝑇𝑃 𝐹𝑎𝑙𝑠𝑒 𝑁𝑒𝑔𝑎𝑡𝑖𝑣𝑒 𝑅𝑎𝑡𝑒= 1 −𝑅𝑒𝑐𝑎𝑙𝑙 **(9)** 

**F1-Scor** e. It is the harmonic mean of Precision and Recall. Unlike arithmetic averaging, the harmonic mean penalizes classifiers that achieve high performance on one metric at the expense of the other, a system that detects every frame as human-occupied achieves perfect Recall but near-zero Precision, and the F1-Score exposes this imbalance. The F1-Score therefore serves as the primary model selection criterion for comparing the SVM-based and rule-based approaches in Objective 4, as it provides a single balanced 

summary statistic robust to the class distribution effects inherent in scenario-based evaluation. 

(𝑃𝑟𝑒𝑐𝑖𝑠𝑖𝑜𝑛)(𝑅𝑒𝑐𝑎𝑙𝑙) 𝐹1 = 2 **(10)** 𝑃𝑟𝑒𝑐𝑖𝑠𝑖𝑜𝑛+ 𝑅𝑒𝑐𝑎𝑙𝑙 

### **Table 8** 

### **_Acceptable Thresholds_** 

**ISO/IEC 25010 SubAcceptance Instrument Characteristic characteristic Threshold for Sarwave** Functional Suitability Functional Aggregate detection 9-Scenario Confusion Correctness accuracy ≥ 85% across Matrix all 9 scenarios 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

86 

**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** Reliability Fault Tolerance & Accuracy std. dev. ≤ Repeated Trial Std. Availability ±5 pp per scenario Dev. Analysis group; zero pipeline failures during testing Performance Efficiency Time Behaviour Mean end-to-end Frame Latency latency ≤ 100 ms; no Profiler on single frame exceeds Raspberry Pi 5 200 ms 

**Reliability.** In the ISO/IEC 25010:2023 sense encompasses the system’s ability to perform its specified detection function consistently without failure across repeated trials under both identical and varied conditions. For Sarwave, it is operationalized through two complementary measures. Intra-scenario reliability is quantified as the standard deviation of Accuracy and FNR values across a minimum of three independent repeated sessions per scenario under identical conditions. The acceptance criterion of SD ≤ ±0.05 is established as a conservative operational threshold. Classification performance estimates for binary systems with 100 samples inherently carry error bars of approximately ±10 percentage points; a trial-to-trial stability criterion of half that magnitude, ±5 pp, therefore represents a meaningful and non-trivial repeatability floor for a safety-critical field device. Interscenario reliability is assessed by evaluating whether performance degrades gracefully, rather than erratically and monotonically, as environmental conditions changes from open to debris to smoke-filled. 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

87 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

**Localization.** This quantifies the degree to which centroid-based position estimates approximate the physical ground truth positions of human subjects. It is computed exclusively on true positive frames, frames where the system correctly classified human presence, because centroid estimation from an incorrect detection provides no meaningful positional data. The Localization Error (LE) for each true positive frame is: 2 𝐿𝐸𝑖 =<sup>√</sup> [(𝑥<sup>𝑒</sup> 𝑠𝑡 − 𝑥<sup>𝑔</sup> 𝑡)<sup>2</sup> + (𝑦<sup>𝑒</sup> 𝑠𝑡<sup>− 𝑦𝑔</sup> 𝑡<sup>)</sup> + (𝑧𝑒𝑠𝑡 − 𝑧𝑔𝑡)2] **(11)** Where 𝑥𝑒𝑠𝑡, 𝑦𝑒𝑠𝑡, and 𝑧𝑒𝑠𝑡are the estimated Cartesian coordinates of the detected subject derived from the DBSCAN cluster centroid, 𝑥𝑔𝑡, 𝑦𝑔𝑡, and 𝑧𝑔𝑡are the corresponding ground truth coordinates measured from the radar sensor origin, and 𝐿𝐸𝑖is the Euclidean distance error in meters for the 𝑖-th true positive frame. Computing the straight-line spatial divergence between tracked radar centroids and ground-truth coordinates using this equation is a standard validation practice found in recent FMCW radar tracking literature (Bannur et al., 2026) Mean LE and standard deviation are reported per zone and aggregated. The practical acceptance threshold of ≤ 1.0 m mean aggregate error is adopted as sufficient directional guidance for rescue personnel operating in smoke-obscured environments and is deliberately conservative relative to the high-precision target-detection and environmental tracking limits reported in comparable ultra-wideband (UWB) search-and-rescue radar system literature (Uzunidis et al., 2024). 

|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE|
|---|



88 

|**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE**|
|---|
|Estimated subject position is derived from the centroid of the DBSCAN cluster<br>associated with the detected human target. Ground-truth coordinates are measured relative<br>to the radar sensor origin using a calibrated laser distance measurer aligned to the radar<br>coordinate frame. All localization error calculations are performed within this common<br>Cartesian reference system.<br>**Functional Correctness**assesses whether Sarwave produces outputs that correctly<br>correspond to their specification, that is, whether the binary SVM classifier correctly<br>identifies human presence and absence across all nine test configurations. This is the<br>broadest indicator of whether the system does what it was designed to do. The aggregate<br>detection accuracy derived from the full 9-scenario confusion matrix is the primary<br>instrument for this assessment. A system achieving the ≥ 85% accuracy threshold across all<br>scenarios demonstrates that its classification function is operationally correct under the full<br>range of tested conditions, from open to smoke-filled environments.|
|**Performance Efficiency.**Benchmarking an average frame processing latency<br>below100ms is a verified benchmark established in recent millimeter-wave (mmWave)<br>edge-computing radar architectures; Abdullah and Patel (2025) demonstrated that a<br>lightweight convolutional neural network (CNN) pipeline handling raw frequency-<br>modulated continuous-wave (FMCW) radar frames can successfully achieve an average<br>localized inference latency of87ms when deployed directly on an embedded Raspberry Pi<br>platform. Processing Time and Detection Time are monitored using a hardware-isolated|
|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE <br>89|



### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

Frame Latency Profiler running natively on the Raspberry Pi 5 via Python's time.perf_counter() high-precision timestamp instrumentation. Processing Time is 

computed as the cumulative latency of three sequential pipeline stages per frame, expressed 

as: 

𝑃𝑟𝑜𝑐𝑒𝑠𝑠𝑖𝑛𝑔 𝑇𝑖𝑚𝑒= 𝑡𝑖𝑛𝑔𝑒𝑠𝑡𝑖𝑜𝑛 + 𝑡𝑝𝑟𝑜𝑐𝑒𝑠𝑠𝑖𝑛𝑔 + 𝑡𝑖𝑛𝑡𝑒𝑟𝑓𝑒𝑟𝑒𝑛𝑐𝑒 **(12)** 

Where 𝑡𝑖𝑛𝑔𝑒𝑠𝑡𝑖𝑜𝑛 represents the data ingestion latency from the IWR6843AOPEVM 

serial UART interface at 921,600 bps, derived from the frame packet transmission formula: 



Where Packet Size is in bits, and Baud Rate in bits per second. 

While 𝑡𝑝𝑟𝑜𝑐𝑒𝑠𝑠𝑖𝑛𝑔 represents the DBSCAN spatial clustering latency executed on the Broadcom BCM2712 quad-core processor at O(N log N) complexity for N detected points per frame, and t_inference represents the SVM classification latency evaluated as a single algebraic decision function: 



Where 𝑠𝑖𝑔𝑛(… ) is a function that returns either a positive or negative. 𝑤𝑖 is the 

weight of the current feature, 𝑥𝑖 is the value of the current input feature, and b is the bias 

term. 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

90 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

Detection Time is derived from a sliding-window verification buffer of 15 to 20 consecutive frames, where a positive survivor alert is only confirmed if the SVM sustains a continuous human-presence classification across the entire buffer window, expressed as: 

𝐷𝑒𝑡𝑒𝑐𝑡𝑖𝑜𝑛 𝑇𝑖𝑚𝑒=<sup>𝑉𝑒𝑟𝑖𝑓𝑖𝑐𝑎𝑡𝑖𝑜𝑛 𝑊𝑖𝑛𝑑𝑜𝑤</sup> **(15)** 𝐹𝑟𝑎𝑚𝑒 𝑅𝑎𝑡𝑒 

Where Verification Window is in frames, and Frame Rate is in frames per second. 

This multi-frame accumulation methodology is empirically validated by Tateoka and Kidera (2026) as necessary to reliably capture the cyclic rhythm of thoracic respiratory micro-motions before rendering a definitive localization output. The engineered targets of 30–70 ms for Processing Time and 1.0–2.0 seconds for Detection Time are established as mathematical design boundaries derived from the hardware pipeline specifications and frame rate constraints of the sensor configuration, against which measured values will be evaluated during system testing. 

### **Baseline Comparison and Statistical Validation** 

A rule-based threshold classifier is implemented as a comparison baseline, classifying a frame as human present if point density exceeds a fixed threshold determined empirically from the training data as the point density value maximizing F1 score on the validation split. This baseline is evaluated on the identical 9-scenario test set under identical conditions as the SVM. Accuracy, FPR, FNR, and F1 score are reported side-by-side per 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

91 

**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** scenario to determine whether machine learning provides a measurable advantage over simple threshold classification for this SAR detection task. Statistical significance of any performance difference between the SVM classifier and the rule-based point density threshold baseline is assessed using McNemar’s test, which is recognized as the standard robust metric for comparing supervised classification algorithms on fixed evaluation sets. McNemar’s test is applied only on the full aggregated nine-scenario results (900 total frames), not on individual scenarios. Per-scenario application with 100 frames each produces too few disagreement cells (b + c) to generate a reliable chi-squared result; the test approximation degrades significantly when the total number of discordant cells falls below approximately 25. McNemar’s test becomes unreliable when b + c falls below approximately 25. At the aggregate level of 900 frames, sufficient disagreement cells accumulate for a meaningful statistical comparison. McNemar’s test is additionally applied in isolation to Scenarios 7–9 (smoke-filled, 300 frames) as a secondary targeted comparison for the most operationally critical condition subset. To adjust for low cell frequencies within sub-scenarios, Edwards’ continuity correction is integrated into the test statistic (Edwards, 1948): 



### COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

92 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

### **REFERENCES** 

Abdelhamid, M., Safa, A., Ismail, L., & Mohamed, A. (2025). FMCW radar for human detection in collapsed structures for post-disaster search and rescue. In 2025 International Wireless Communications and Mobile Computing (IWCMC) (pp. 1638–1643). IEEE. https://doi.org/10.1109/IWCMC65282.2025.11059497 Abdullah, D., & Patel, P. (2025). Real-time gesture recognition using mmWave radar signal processing and CNN models. National Journal of Signal and Image Processing, 1(3), 15–22. https://doi.org/10.17051/NJSIP/01.03.03 Ahmed, S., & Cho, S. H. (2023). Machine learning for healthcare radars: Recent progresses in human vital sign measurement and activity recognition. IEEE Communications Surveys & Tutorials, 26(1), 461–495. https://doi.org/10.1109/COMST.2023.3334269 

Ali, M. W., Gupta, A., Khan, M., & Wajid, M. (2024). Non-contact breath rate classification using SVM model and mmWave radar sensor data. In 2024 2nd International Conference on Cyber Physical Systems, Power Electronics and Electric Vehicles (ICPEEV) (pp. 1–6). IEEE. 

https Amershi, S., Begel, A., Bird, C., DeLine, R., Gall, H., Kamar, E., Nagappan, N., Nushi, B., & Zimmermann, T. (2019). Software Engineering for Machine 

Learning: A Case Study. _International Conference on Software Engineering (ICSE 2019) - Software Engineering in Practice Track_ , 291–300. https://doi.org/10.1109/icse-seip.2019.00042 

### COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

93 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

://doi.org/10.1109/ICPEEV63032.2024.10931988 Bounmy, S., & Sisavath, K. (2023). Securing Internet of Things (IoT) Ecosystems: A Quantum Cryptography approach. Algorithm Asynchronous, 1(1), 1–7. https://doi.org/10.61963/jaa.v1i1.45 

Chan, Q. N., Gao, D., Zhou, Y., Xing, S., Zhai, G., Wang, C., Wang, W., Lim, S. H., Lee, E. W. M., & Yeoh, G. H. (2024). A novel movable mannequin platform for evaluating and optimising mmWave radar sensor for indoor crowd evacuation monitoring applications. Fire, 7(6), 181. https://doi.org/10.3390/fire7060181 Chang, Z., Zhang, F., Ma, X., Wang, P., Chen, W., Duo, Z., Jouaber, B., & Zhang, D. (2024). MmECare: Enabling fine-grained vital sign monitoring for emergency care with handheld mmWave radars. Proceedings of the ACM on Interactive, Mobile, Wearable and Ubiquitous Technologies, 8, 1–24. https://doi.org/10.1145/3699766 

Chen, C., Yao, Z., Jiang, J., Pan, X., He, X., Chen, Z., & Wang, B. (2024). SMOkeNAV: Millimeter-wave-radar/inertial measurement unit integrated positioning and semantic mapping in visually degraded environments for first responders. Advanced Intelligent Systems, 6(12). https://doi.org/10.1002/aisy.202400241 Chen, Y., Yuan, J., & Tang, J. (2024). A high precision vital signs detection method based on millimeter wave radar. Scientific Reports, 14(1), 25535. https://doi.org/10.1038/s41598-024-77683-1 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

94 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

Chicco, D., & Jurman, G. (2020). The advantages of the Matthews correlation coefficient (MCC) over F1 score and accuracy in binary classification evaluation. _BMC Genomics_ , _21_ (1), 6. https://doi.org/10.1186/s12864-019-6413-7 Hosseini, N., Khatun, M., Guo, C., Du, K., Ozdemir, O., Matolak, D. W., Guvenc, I., & Mehrpouyan, H. (2021). Attenuation of several common building materials: Millimeter-Wave frequency bands 28, 73, and 91 GHz. _IEEE Antennas and Propagation Magazine_ , _63_ (6), 40–50. https://doi.org/10.1109/map.2020.3043445 Fusco, A., Sakharov, S., Lavronenko, K., Hazra, S., Servadei, L., & Wille, R. (2024). Deep learning classifier for robust artifact rejection in FMCW radar vital sensing. IEEE Sensors Journal, 25(13), 23304–23311. https://doi.org/10.1109/JSEN.2024.3497797 Ham, A. G., Nafornita, C., Vesa, V. C., Copacean, G., Davidovici, V. D., & Nafornita, I. (2025). Grid search and genetic algorithm optimization of neural networks for automotive radar object classification. _Sensors_ , _25_ (19), 6017. https://doi.org/10.3390/s25196017 

Han, T. T., Pham, H. Y., Nguyen, D. S. L., Iwata, Y., Do, T. T., Ishibashi, K., & Sun, G. (2021). Machine learning based classification model for screening of infected patients using vital signs. Informatics in Medicine Unlocked, 24, 100592. https://doi.org/10.1016/j.imu.2021.100592 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

95 

**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** International Organization for Standardization & International Electrotechnical Commission. (2011). _ISO/IEC 25010:2011: Systems and software engineering — Systems and software Quality Requirements and Evaluation (SQuaRE) — System and software quality models_ . ISO/IEC. 

Jing, Y., Yan, Y., Li, Z., Qi, F., Lei, T., Wang, J., & Lu, G. (2025). Advancing remote life sensing for search and rescue: A novel framework for precise vital signs detection via airborne UWB radar. Sensors, 25(17), 5232. https://doi.org/10.3390/s25175232 Joo, K., & Seo, J. (2025). Quantitative simulation of human evacuation dynamics under visibility impairment in indoor fire scenarios. Architecture Image Studies, 6(4). https://doi.org/10.62754/ais.v6i4.409 Kasnesis, P., Chatzigeorgiou, C., Doulgerakis, V., Uzunidis, D., Margaritis, E., Patrikakis, C., & Mitilineos, S. (2024). RadIOCD: Radar-based Interior Object Classification Dataset [Data set]. Zenodo. https://zenodo.org/records/10731407 Kebe, M., Gadhafi, R., Mohammad, B., Sanduleanu, M., Saleh, H., & Al-Qutayri, M. (2020). Human vital signs detection methods and potential using radars: A review. Sensors, 20(5), 1454. https://doi.org/10.3390/s20051454 Kitchenham, B., & Pfleeger, S. L. (1996). Software quality: The elusive target. _IEEE Software_ , 13(1), 12–21. https://doi.org/10.1109/52.476281 

Lagata, L. S., Andujar, C. J., Lantaco, E. L. S., Manuales, K. C. A., Allanic, E. A., & 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

96 

**LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** Cuevas, J. F., Jr. (2022). Challenges encountered and insights of the Bureau of Fire Protection personnel towards responding fire incident. Mediterranean Journal of Basic and Applied Sciences, 06(02), 103–117. https://doi.org/10.46382/mjbas.2022.6211 Lai, L., & Suda, N. (2018). Rethinking machine learning development and deployment for edge devices. _arXiv (Cornell University)_ . https://doi.org/10.48550/arxiv.1806.07846 Li, X., Wang, X., Zhou, P., Cui, X., Xu, Y., & Shi, X. (2023). Design and implementation of a miniature millimeter wave radar system for multiple applications. In 2023 International Conference on Microwave and Millimeter Wave Technology (ICMMT) (pp. 1–3). IEEE. https://doi.org/10.1109/ICMMT58241.2023.10277081 Liu, H., Wang, Y., Zhou, M., Wang, D., Xie, L., & Nie, W. (2023). Millimeter-wave radar vital signs detection based on modified independent component analysis. In 2023 IEEE 11th Asia-Pacific Conference on Antennas and Propagation (APCAP) (pp. 1–2). IEEE. https://doi.org/10.1109/APCAP59480.2023.10470254 Liu, L., Zhang, J., Qu, Y., Zhang, S., & Xiao, W. (2023). mmRH: Noncontact vital sign detection with an FMCW mm-wave radar. IEEE Sensors Journal, 23(8), 8856– 8866. https://doi.org/10.1109/JSEN.2023.3250500 Liu, W., Zhang, S., Yang, J., & Wang, M. (2023). Human vital signs detection based on millimeter wave radar and digital filtering. In 2023 IEEE International Conference on Image Processing and Computer Applications (ICIPCA) (pp. 569–574). IEEE. 

|COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE|
|---|



97 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

https://doi.org/10.1109/ICIPCA59209.2023.10257847 Lu, C. X., Rosa, S., Zhao, P., Wang, B., Chen, C., Stankovic, J. A., Trigoni, N., & Markham, A. (2020). See Through Smoke: Robust Indoor Mapping with Low-cost mmWave Radar. _arXiv (Cornell University)_ . https://doi.org/10.48550/arxiv.1911.00398 Ma, J., Chen, H., Wang, Y., Shi, J., Shi, Q., & Xu, Z. (2026). First demonstration of human-portable millimeter wave SAR imaging with motion-sensor-free autofocus for search and rescue applications. IEEE Transactions on Aerospace and Electronic Systems, 62, 2550–2565. https://doi.org/10.1109/TAES.2026.3654915 Marty, S., Pantanella, F., Ronco, A., Dheman, K., & Magno, M. (2023). Investigation of mmWave radar technology for non-contact vital sign monitoring. In 2023 IEEE International Symposium on Medical Measurements and Applications (MeMeA) (pp. 1–6). IEEE. https://doi.org/10.1109/MeMeA57477.2023.10171940 Michalopoulos, A., Paliodimos, E. N., Papadopoulos, F., Nikolaou, G., Patrikakis, C., & Mytilinaios, S. A. (2025). Victim detection using a robot-mounted UWB-radar platform. In 14th International Conference on Modern Circuits and Systems Technologies (pp. 1–4). IEEE. https://doi.org/10.1109/mocast65744.2025.11083953 N, D. B., S, V. G., Srihari, P., & Pardhasaradhi, B. (2022). Real Time Vital Sign Monitoring System using AWR1642 Radar Module with Remote Access. 2022 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

98 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

IEEE International Symposium On Smart Electronic Systems (iSES), 191–195. https://doi.org/10.1109/ises54909.2022.00047 

Ndrecaj, J., Berisha, S., & Çunaku, E. (2023). An approach with iterative and incremental development (IID) for mobile applications. In _Advances in wireless technologies and telecommunication book series_ (pp. 67–81). https://doi.org/10.4018/978-16684-8582-8.ch005 

Niyaz, Ö., Tüylü, T., Mahouti, P., et al. (2024). Detection and classification of human respiration under building debris model using VHF/UHF waves. Neural Computing and Applications, 36, 14709–14725. https://doi.org/10.1007/s00521024-09848-y 

Nocera, A., Senigagliesi, L., Raimondi, M., Ciattaglia, G., & Gambi, E. (2024). Machine learning in RADAR-based physiological signals sensing: A scoping review of the models, datasets, and metrics. IEEE Access, 12, 156082–156117. https://doi.org/10.1109/ACCESS.2024.3482690 

Powers, D. M. W. (2011). Evaluation: From precision, recall and F-measure to ROC, informedness, markedness and correlation. _Journal of Machine Learning Technologies_ , 2(1), 37–63. 

Prabhakara, A., Jin, T., Das, A., Bhatt, G., Kumari, L., Soltanaghai, E., Bilmes, J., Swarun Kumar, & Anthony Rowe. (n.d.). High Resolution Point Clouds from mmWave Radar. In Carnegie Mellon University, University of Washington, University of 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

99 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

Illinois, Urbana-Champaign, & Bosch Research, _Carnegie Mellon University_ [Journal-article]. https://akarsh-prabhakara.github.io/files/radarhd-icra23.pdf Pramanik, S. K., & Islam, S. M. M. (2024). Through the wall human heart beat detection using single channel CW radar. Frontiers in Physiology, 15, 1344221. https://doi.org/10.3389/fphys.2024.1344221 Rahman, F. S., Tannous, W. K., Avsar, G., Agho, K. E., Ghassempour, N., & Harvey, L. A. (2023). Economic costs of residential fires: A systematic review. Fire, 6(10), 399. https://doi.org/10.3390/fire6100399 Richey, R. C., & Klein, J. D. (2005). Developmental research methods: Creating knowledge from instructional design and development practice. _Journal of Computing in Higher Education, 16_ (2), 23–38. https://doi.org/10.1007/BF02961473 Rohman, B. P. A., Rudrappa, M. T., Shargorodskyy, M., Herschel, R., & Nishimoto, M. (2021). Moving human respiration sign detection using mm-wave radar via motion path reconstruction. In 2021 International Conference on Radar, Antenna, Microwave, Electronics, and Telecommunications (ICRAMET) (pp. 196–200). IEEE. https://doi.org/10.1109/ICRAMET53537.2021.9650479 Schenkel, F., Schultze, T., Baer, C., Rolfes, I., & Schulz, C. (2024). Radar-enabled millimeter-wave sensing of fire interactions. IEEE Transactions on Instrumentation and Measurement, 73, Article 8003810. https://doi.org/10.1109/TIM.2024.3400306 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

100 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

Shi, D., Liang, F., Qiao, J., Wang, Y., Zhu, Y., Lv, H., Yu, X., Jiao, T., Liao, F., Yan, K., et al. (2023). A novel non-contact detection and identification method for the postdisaster compression state of injured individuals using UWB bio-radar. 

Bioengineering, 10(8), 905. https://doi.org/10.3390/bioengineering10080905 Shi, Y., & Shi, J. (2026). Mask-aware spatiotemporal classification of millimeter-wave radar point cloud sequences using DGCNN and Transformer for child–pet recognition in enclosed spaces. Sensors, 26(5), 1580. https://doi.org/10.3390/s26051580 Sokolova, M., & Lapalme, G. (2009). A systematic analysis of performance measures for classification tasks. _Information Processing & Management_ , _45_ (4), 427–437. https://doi.org/10.1016/j.ipm.2009.03.002 

Starnes, A. (2021, March 5). Enhanced search methodology—Making the rescue up to 70% faster. Insight Training LLC. https://insighttrainingllc.com/uncategorized/enhanced-searchmethodologymaking-the-rescue-up-to-70-faster/ Starr, J. W., & Lattimer, B. Y. (2013). Evaluation of navigation sensors in fire smoke environments. Fire Technology, 50(6), 1459–1481. https://doi.org/10.1007/s10694-013-0356-3 Tang, W. (2024). The application of FMCW radar technology in human heartbeat respiration monitoring. Highlights in Science, Engineering and Technology, 119, 249–253. https://doi.org/10.54097/tjz8gp64 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

101 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

Tateoka, T., & Kidera, S. (2026). Millimeter wave radar-based survivor detection and localization in indoor rescue scenarios. IEEE Open Journal of Antennas and Propagation, 7(2), 498–511. https://doi.org/10.1109/OJAP.2026.3652632 Uzunidis, D., Kasnesis, P., Papadopoulos, F. G., Paliodimos, E. N., Patrikakis, C. Z., & Mitilineos, S. A. (2024). Breathing rate estimation behind walls exploiting a UWB radar sensor. In 2024 13th International Conference on Modern Circuits and Systems Technologies (MOCAST) (pp. 1–4). IEEE. https://doi.org/10.1109/MOCAST61810.2024.10615864 Uzunidis, D., Kasnesis, P., Patrikakis, C. Z., & Mitilineos, S. A. (2024). Machine learning-based human life detection behind walls exploiting a UWB radar sensor. Preprints.org. https://doi.org/10.20944/preprints202403.0271.v1 Uzunidis, D., Mitilineos, S. A., Ponti, C., Schettini, G., & Patrikakis, C. Z. (2023). Detection of trapped victims behind large obstacles using radar sensors: A review on available technologies and candidate solutions. In 2023 IEEE Conference on Antenna Measurements and Applications (CAMA) (pp. 1025–1030). IEEE. https://doi.org/10.1109/CAMA57522.2023.10352910 Wagner, S. (2013). _Software product quality control_ . Springer. https://doi.org/10.1007/978-3-642-38571-1 Wang, H., Yang, Y., Zhong, X., & Ling, Z. (2026). Multi-window Gabor transform network for ground penetrating radar B-Scan image reconstruction. Proceedings of the AAAI Conference on Artificial Intelligence, 40(12), 9829–9837. 

### COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

102 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

https://doi.org/10.1609/aaai.v40i12.37947 

Wang, S., & Weng, W. (2023). Dilemmas and directions of real-time detecting trapped individuals using ultra-wideband radar in building fire rescue. https://doi.org/10.1515/freq-2022-0237 

Wang, S., Xu, P., Weng, W., Niu, L., & Wang, R. (2024). An end-to-end recognition method for IR-UWB radar dynamic detection mode for detecting targets in fire 

rescue scenarios. IEEE Internet of Things Journal, 11(23), 38137–38150. https://doi.org/10.1109/JIOT.2024.3443865 

Wu, Y., Ni, H., Mao, C., Han, J., & Xu, W. (2023). Non-intrusive human vital sign detection using mmWave sensing technologies: A review. ACM Transactions on Sensor Networks, 20(1), 1–36. https://doi.org/10.1145/3627161 

Wu, Y., Zhou, Z., Jiang, Z., Han, J., & Xu, W. (2026). SkeletonHR: Robust heart rate detection under movement conditions using a commodity mmWave radar sensor. IEEE Sensors Journal, 26(6), 8923–8935. 

https://doi.org/10.1109/JSEN.2026.3654963 

Xie, M., Du, X., Chen, Y., Li, X., Zhao, M., & Ji, J. (2026). A noise reduction algorithm for FMCW radar vital signs signals. Measurement, 278, 121592. https://doi.org/10.1016/j.measurement.2026.121592 

Yeom, S. (2024). Thermal image tracking for search and rescue missions with a drone. Drones, 8(2), 53. https://doi.org/10.3390/drones8020053 

### COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

103 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

Yılmaz, B., Öz, F., Erer, I., & Baykut, S. (2024). Detection of vital signs in debris environment using SFCW radar. In 2024 32nd Signal Processing and Communications Applications Conference (SIU) (pp. 1–4). IEEE. https://doi.org/10.1109/SIU61531.2024.10601146 

Yin, J., Cao, Z., Pan, B., Lv, S., & Cui, Z. (2024). Enhanced vital sign monitoring in multi-target environments: A FMCW radar approach with blind source separation. In IGARSS 2024 – 2024 IEEE International Geoscience and Remote Sensing Symposium (pp. 9530–9534). IEEE. https://doi.org/10.1109/IGARSS53475.2024.10640374 

Zhang, J., Wu, J., Hu, H., Wang, M., & Liu, X. (2024). Application of millimeter-wave radar in the life detection system of the mining area. In Proceedings of the 3rd International Conference on Electronic Information Technology and Smart Agriculture (ICEITSA '23) (pp. 57–61). ACM. https://doi.org/10.1145/3641343.3641353 

Zhao, P., Lu, C. X., Wang, J., Chen, C., Wang, W., Trigoni, N., & Markham, A. (2021). Human tracking and identification through a millimeter wave radar. Ad Hoc Networks, 116, 102475. https://doi.org/10.1016/j.adhoc.2021.102475 

Zhao, Z., et al. (2020). Point cloud features-based kernel SVM for human-vehicle classification in millimeter wave radar. IEEE Access, 8, 26012–26021. https://doi.org/10.1109/ACCESS.2020.2970533 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

104 

### **LYCEUM OF THE PHILIPPINES UNIVERSITY - CAVITE** 

Zheng, J., fWen, P., Chen, J., Yang, M., & Zhang, S. (2025). Variational modal decomposition based on whale optimization algorithm for vital signs detection of FMCW radar. In Proceedings of the 16th International Conference on Signal Processing Systems (Vol. 13559). SPIE. https://doi.org/10.1117/12.3060566 

COLLEGE OF ENGINEERING, COMPUTER STUDIES AND ARCHITECTURE 

105 

