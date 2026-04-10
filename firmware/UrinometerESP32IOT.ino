#include <WiFi.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HX711_ADC.h>
#include <RTClib.h>

// WiFi Credentials
const char* ssid = "ashwin";
const char* password = "12345678";
//const float MyCalibrationFactor = 258.9333333333;
const float MyCalibrationFactor = 266.6238095238;


WiFiServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

const char* htmlPage = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Medical Urinometer</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <link href="https://fonts.googleapis.com/css2?family=Roboto:wght@300;400;500;700&display=swap" rel="stylesheet">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Roboto', sans-serif;
        }
        
        body {
            background-color: #f0f5ff;
            color: #2c3e50;
            padding: 20px;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        
        header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 15px 20px;
            background: linear-gradient(135deg, #4a90e2, #63b3ed);
            color: white;
            border-radius: 10px;
            margin-bottom: 20px;
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
        }
        
        .logo {
            display: flex;
            align-items: center;
            gap: 15px;
        }
        
        .logo i {
            font-size: 2.5rem;
        }
        
        .logo h1 {
            font-size: 1.8rem;
            font-weight: 700;
        }
        
        .patient-info {
            text-align: right;
            font-size: 0.9rem;
        }
        
        .dashboard {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            margin-bottom: 20px;
        }
        
        .card {
            background: white;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.05);
        }
        
        .card-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 15px;
            padding-bottom: 10px;
            border-bottom: 1px solid #eaeaea;
        }
        
        .card-header h2 {
            font-size: 1.2rem;
            color: #4a5568;
            font-weight: 500;
        }
        
        .card-header i {
            font-size: 1.5rem;
            color: #4a90e2;
        }
        
        .stats {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
        }
        
        .stat-box {
            text-align: center;
            padding: 15px;
            border-radius: 8px;
            background: linear-gradient(135deg, #f8f9fa, #e9ecef);
        }
        
        .stat-value {
            font-size: 2rem;
            font-weight: 700;
            margin: 10px 0;
            color: #2c3e50;
        }
        
        .stat-label {
            font-size: 0.9rem;
            color: #718096;
        }
        
        .chart-container {
            height: 300px;
            position: relative;
        }
        
        .full-width {
            grid-column: 1 / -1;
        }
        
        .interpretation {
            background: linear-gradient(135deg, #fff5f5, #fed7d7);
            padding: 15px;
            border-radius: 8px;
            margin-top: 15px;
        }
        
        .interpretation h3 {
            color: #c53030;
            margin-bottom: 10px;
            font-size: 1.1rem;
        }
        
        .history-table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 15px;
        }
        
        .history-table th, .history-table td {
            padding: 12px 15px;
            text-align: left;
            border-bottom: 1px solid #eaeaea;
        }
        
        .history-table th {
            background-color: #f8f9fa;
            font-weight: 500;
            color: #4a5568;
        }
        
        .history-table tr:hover {
            background-color: #f8f9fa;
        }
        
        .status-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
        }
        
        .normal {
            background-color: #48bb78;
        }
        
        .warning {
            background-color: #ecc94b;
        }
        
        .critical {
            background-color: #f56565;
        }
        
        .buttons {
            display: flex;
            gap: 15px;
            margin-top: 20px;
        }
        
        .btn {
            padding: 12px 20px;
            border: none;
            border-radius: 6px;
            font-weight: 500;
            cursor: pointer;
            display: flex;
            align-items: center;
            gap: 8px;
            transition: all 0.3s ease;
        }
        
        .btn-primary {
            background: linear-gradient(135deg, #4a90e2, #63b3ed);
            color: white;
        }
        
        .btn-secondary {
            background: #e2e8f0;
            color: #4a5568;
        }
        
        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        }
        
        footer {
            text-align: center;
            margin-top: 30px;
            padding: 20px;
            color: #718096;
            font-size: 0.9rem;
        }
        
        @media (max-width: 768px) {
            .dashboard {
                grid-template-columns: 1fr;
            }
            
            .stats {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <div class="logo">
                <i class="fas fa-tint"></i>
                <div>
                    <h1>Medical Urinometer</h1>
                    <p>Real-time Urine Flow Analysis</p>
                </div>
            </div>
            <div class="patient-info">
                <p>Patient: <strong>John Doe</strong> (ID: 12345)</p>
                <p>Date: <span id="current-date">November 5, 2023</span></p>
            </div>
        </header>
        
        <div class="dashboard">
            <div class="card">
                <div class="card-header">
                    <h2><i class="fas fa-weight"></i> Weight Measurement</h2>
                    <i class="fas fa-weight"></i>
                </div>
                <div class="stats">
                    <div class="stat-box">
                        <div class="stat-label">Current Weight</div>
                        <div class="stat-value" id="current-weight">0.0 g</div>
                        <div class="stat-label">Bladder Content</div>
                    </div>
                    <div class="stat-box">
                        <div class="stat-label">Total Volume</div>
                        <div class="stat-value" id="total-volume">0.0 ml</div>
                        <div class="stat-label">Cumulative</div>
                    </div>
                </div>
            </div>
            
            <div class="card">
                <div class="card-header">
                    <h2><i class="fas fa-wind"></i> Flow Rate</h2>
                    <i class="fas fa-wind"></i>
                </div>
                <div class="stats">
                    <div class="stat-box">
                        <div class="stat-label">Current Flow Rate</div>
                        <div class="stat-value" id="current-flow">0.0 g/s</div>
                        <div class="stat-label">Real-time</div>
                    </div>
                    <div class="stat-box">
                        <div class="stat-label">Peak Flow Rate</div>
                        <div class="stat-value" id="peak-flow">0.0 g/s</div>
                        <div class="stat-label">Maximum</div>
                    </div>
                </div>
            </div>
            
            <div class="card full-width">
                <div class="card-header">
                    <h2><i class="fas fa-chart-line"></i> Real-time Monitoring</h2>
                    <i class="fas fa-chart-line"></i>
                </div>
                <div class="chart-container">
                    <canvas id="flow-chart"></canvas>
                </div>
            </div>
            
            <div class="card">
                <div class="card-header">
                    <h2><i class="fas fa-clipboard-list"></i> Clinical Interpretation</h2>
                    <i class="fas fa-clipboard-list"></i>
                </div>
                <div id="interpretation-container">
                    <p>Waiting for sufficient data to provide analysis...</p>
                </div>
                <div class="interpretation">
                    <h3><i class="fas fa-exclamation-triangle"></i> Clinical Notes</h3>
                    <p>Normal urinary flow rate typically ranges between 10-21 ml/sec for adults. Values consistently outside this range may indicate underlying conditions that require further investigation.</p>
                </div>
            </div>
            
            <div class="card">
                <div class="card-header">
                    <h2><i class="fas fa-history"></i> Session History</h2>
                    <i class="fas fa-history"></i>
                </div>
                <table class="history-table">
                    <thead>
                        <tr>
                            <th>Time</th>
                            <th>Weight (g)</th>
                            <th>Flow Rate (g/s)</th>
                            <th>Status</th>
                        </tr>
                    </thead>
                    <tbody id="history-body">
                        <tr>
                            <td colspan="4" style="text-align: center;">No data recorded yet</td>
                        </tr>
                    </tbody>
                </table>
                
                <div class="buttons">
                    <button class="btn btn-primary" onclick="exportData()">
                        <i class="fas fa-download"></i> Export Data
                    </button>
                    <button class="btn btn-secondary" onclick="window.print()">
                        <i class="fas fa-print"></i> Print Report
                    </button>
                </div>
            </div>
        </div>
        
        <footer>
            <p>Medical Urinometer System | For clinical use only | Data updates every 2 seconds</p>
            <p>&copy; 2023 Medical Diagnostics Inc. | Version 2.1</p>
        </footer>
    </div>

    <script>
        // Initialize variables
        let weightData = [];
        let flowRateData = [];
        let labels = [];
        let peakFlowRate = 0;
        let totalVolume = 0;
        let chart;
        
        // DOM elements
        const currentWeightEl = document.getElementById('current-weight');
        const totalVolumeEl = document.getElementById('total-volume');
        const currentFlowEl = document.getElementById('current-flow');
        const peakFlowEl = document.getElementById('peak-flow');
        const historyBodyEl = document.getElementById('history-body');
        const interpretationContainerEl = document.getElementById('interpretation-container');
        const currentDateEl = document.getElementById('current-date');
        
        // Set current date
        currentDateEl.textContent = new Date().toLocaleDateString('en-US', { 
            year: 'numeric', 
            month: 'long', 
            day: 'numeric' 
        });
        
        // Initialize chart
        function initChart() {
            const ctx = document.getElementById('flow-chart').getContext('2d');
            chart = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: labels,
                    datasets: [
                        {
                            label: 'Weight (g)',
                            borderColor: 'rgb(54, 162, 235)',
                            backgroundColor: 'rgba(54, 162, 235, 0.1)',
                            data: weightData,
                            fill: true,
                            yAxisID: 'y',
                            tension: 0.3
                        },
                        {
                            label: 'Flow Rate (g/s)',
                            borderColor: 'rgb(255, 99, 132)',
                            backgroundColor: 'rgba(255, 99, 132, 0.1)',
                            data: flowRateData,
                            fill: true,
                            yAxisID: 'y1',
                            tension: 0.3
                        }
                    ]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    interaction: {
                        mode: 'index',
                        intersect: false,
                    },
                    scales: {
                        x: {
                            title: {
                                display: true,
                                text: 'Time'
                            },
                            grid: {
                                display: true,
                                color: 'rgba(0, 0, 0, 0.05)'
                            }
                        },
                        y: {
                            type: 'linear',
                            display: true,
                            position: 'left',
                            title: {
                                display: true,
                                text: 'Weight (g)'
                            },
                            grid: {
                                color: 'rgba(54, 162, 235, 0.1)'
                            }
                        },
                        y1: {
                            type: 'linear',
                            display: true,
                            position: 'right',
                            title: {
                                display: true,
                                text: 'Flow Rate (g/s)'
                            },
                            grid: {
                                drawOnChartArea: false,
                                color: 'rgba(255, 99, 132, 0.1)'
                            }
                        }
                    },
                    plugins: {
                        legend: {
                            position: 'top',
                        },
                        title: {
                            display: true,
                            text: 'Real-time Urine Flow Analysis'
                        }
                    }
                }
            });
        }
        
        // Update interpretation based on data
        function updateInterpretation(weight, flowRate) {
            if (weightData.length < 5) {
                interpretationContainerEl.innerHTML = '<p>Collecting data... Please wait for more measurements.</p>';
                return;
            }
            
            let interpretation = '';
            let status = 'normal';
            
            if (flowRate > 21) {
                interpretation = '<p>Elevated flow rate detected. This may indicate a strong bladder contraction or low urethral resistance.</p>';
                status = 'warning';
            } else if (flowRate < 10 && flowRate > 3) {
                interpretation = '<p>Reduced flow rate observed. This could suggest possible bladder outlet obstruction or detrusor underactivity.</p>';
                status = 'warning';
            } else if (flowRate <= 3 && weight > 50) {
                interpretation = '<p>Significantly reduced flow rate with substantial bladder content. This may indicate urinary retention or significant obstruction.</p>';
                status = 'critical';
            } else {
                interpretation = '<p>Flow rate within normal parameters. Pattern suggests typical voiding function.</p>';
                status = 'normal';
            }
            
            interpretation += '<p>Current flow pattern shows ' + (flowRate > 15 ? 'strong' : 'moderate') + ' flow with ' + (weight > 100 ? 'significant' : 'moderate') + ' bladder content.</p>';
            
            interpretationContainerEl.innerHTML = '<div class="status-indicator ' + status + '"></div><strong>' + 
                status.charAt(0).toUpperCase() + status.slice(1) + ' Flow Pattern</strong>' + interpretation;
        }
        
        // Update history table
        function updateHistory(weight, flowRate) {
            if (historyBodyEl.rows.length === 1 && historyBodyEl.rows[0].cells[0].colSpan === 4) {
                historyBodyEl.deleteRow(0);
            }
            
            const now = new Date();
            const timeString = now.toLocaleTimeString();
            
            let status = 'normal';
            if (flowRate > 21) status = 'warning';
            if (flowRate < 10 && flowRate > 3) status = 'warning';
            if (flowRate <= 3 && weight > 50) status = 'critical';
            
            const row = historyBodyEl.insertRow(0);
            row.innerHTML = '<td>' + timeString + '</td><td>' + weight.toFixed(1) + '</td><td>' + 
                flowRate.toFixed(1) + '</td><td><div class="status-indicator ' + status + 
                '"></div> ' + status.charAt(0).toUpperCase() + status.slice(1) + '</td>';
            
            // Keep only last 10 entries
            if (historyBodyEl.rows.length > 10) {
                historyBodyEl.deleteRow(historyBodyEl.rows.length - 1);
            }
        }
        
        // Export data function
        function exportData() {
            let csvContent = "data:text/csv;charset=utf-8,";
            csvContent += "Time,Weight (g),Flow Rate (g/s)\n";
            
            for (let i = 0; i < weightData.length; i++) {
                csvContent += labels[i] + "," + weightData[i] + "," + flowRateData[i] + "\n";
            }
            
            const encodedUri = encodeURI(csvContent);
            const link = document.createElement("a");
            link.setAttribute("href", encodedUri);
            link.setAttribute("download", "urinometer_data.csv");
            document.body.appendChild(link);
            link.click();
            document.body.removeChild(link);
        }
        
        // WebSocket connection
        var ws = new WebSocket('ws://' + location.hostname + ':81/');
        
        ws.onmessage = function(event) {
            const data = event.data.split(',');
            const weight = parseFloat(data[0]);
            const flowRate = parseFloat(data[1]);
            const time = new Date().toLocaleTimeString();
            
            // Update data arrays
            labels.push(time);
            weightData.push(weight);
            flowRateData.push(flowRate);
            
            // Keep only last 20 data points
            if (labels.length > 20) {
                labels.shift();
                weightData.shift();
                flowRateData.shift();
            }
            
            // Update peak flow rate
            if (flowRate > peakFlowRate) {
                peakFlowRate = flowRate;
            }
            
            // Update total volume (simplified)
            totalVolume += flowRate * 0.5;
            
            // Update UI
            currentWeightEl.textContent = weight.toFixed(1) + ' g';
            totalVolumeEl.textContent = totalVolume.toFixed(1) + ' ml';
            currentFlowEl.textContent = flowRate.toFixed(1) + ' g/s';
            peakFlowEl.textContent = peakFlowRate.toFixed(1) + ' g/s';
            
            // Update chart
            if (chart) {
                chart.update();
            }
            
            // Update interpretation
            updateInterpretation(weight, flowRate);
            
            // Update history
            updateHistory(weight, flowRate);
        };
        
        // Initialize the application
        function init() {
            initChart();
        }
        
        // Start the application when the page loads
        window.onload = init;
    </script>
</body>
</html>
)=====";

void handleClient() {
    WiFiClient client = server.available();
    if (client) {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:text/html");
        client.println("Connection: close");
        client.println();
        client.println(htmlPage);
        client.println();
        client.stop();
    }
}

void handleWebSocketMessage(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_CONNECTED) {
        Serial.printf("Client %u connected\n", num);
    }
}

// Pin definitions for ESP32
#define HX711_dout  4   // HX711 Data pin
#define HX711_sck   5   // HX711 Clock pin
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 // Not used
#define OLED_ADDRESS  0x3C

// Create HX711_ADC object
HX711_ADC loadCell(HX711_dout, HX711_sck);

// Create OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Variables for flow calculation
float prev_weight = 0;
unsigned long prev_time = 0;

void setup() {
    Serial.begin(115200);

    // WiFi Setup
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.begin();
    webSocket.begin();
    webSocket.onEvent(handleWebSocketMessage);

    // OLED Setup
    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
    display.clearDisplay();
    display.display();

    // Load Cell Setup
    loadCell.begin();
    unsigned long stabilizingTime = 2000;  // ms
    boolean doTare = true;                 // perform tare on startup
    loadCell.start(stabilizingTime, doTare);

    // Calibration factor
    loadCell.setCalFactor(MyCalibrationFactor); 

    if (loadCell.getTareTimeoutFlag()) {
        Serial.println("Tare timeout, check wiring!");
    } else {
        Serial.println("HX711 ADC Started and Tare complete...");
    }
}

void loop() {
    handleClient();
    webSocket.loop();

    if (loadCell.update()) {
        float curr_weight = loadCell.getData();
        float flow_rate = (curr_weight - prev_weight) / 0.5;  // g/s
        Serial.print("Weight: ");
        Serial.print(curr_weight);
        Serial.print(" g, Flow Rate: ");
        Serial.print(flow_rate);
        Serial.println(" g/s");

        webSocket.broadcastTXT(String(curr_weight) + "," + String(flow_rate));

        // Update OLED display (large left-aligned text)
        display.clearDisplay();
        display.setTextSize(2);   // Bigger text
        display.setTextColor(SSD1306_WHITE);

        // First line - Weight
        display.setCursor(0, 0);
        display.print("Wt: ");
        display.print(curr_weight, 1);  // 1 decimal place
        display.print(" g");

        // Second line - Flow Rate
        display.setCursor(0, 32);
        display.print("FR: ");
        display.print(flow_rate, 1);
        display.print(" g/s");

        display.display();

        prev_weight = curr_weight;
    }
    delay(1000);  // change this to increase delay
}
