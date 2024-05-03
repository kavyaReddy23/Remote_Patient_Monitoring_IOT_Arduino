<?php
// Start the session
session_start();

// Check if the user is logged in as a patient
if (!isset($_SESSION["user"]) || $_SESSION["user"] == "" || $_SESSION['usertype'] != 'p') {
    header("location: ../index.php");
    exit; // Add exit to prevent further execution
} else {
    $useremail = $_SESSION["user"];
}

// Import the database connection
include("../connection.php");

// Initialize variables
$pname = "";
$room_temp = "";
$humidity = "";
$body_temp = "";
$spo2 = "";
$bpm = "";

// Retrieve data from the database for the current patient
$sql = "SELECT p.pname, pm.room_temp, pm.humidity, pm.body_temp, pm.spo2, pm.bpm
        FROM patient p
        LEFT JOIN patient_monitoring pm ON p.pid = pm.patient_id
        WHERE p.pemail = '$useremail'";
$stmt = $database->prepare($sql);
$stmt->execute();
$result = $stmt->get_result();

// Fetch patient and monitoring data
if ($row = $result->fetch_assoc()) {
    $pname = $row["pname"];
    $room_temp = $row["room_temp"];
    $humidity = $row["humidity"];
    $body_temp = $row["body_temp"];
    $spo2 = $row["spo2"];
    $bpm = $row["bpm"];
} else {
    // Handle case where no data is found
    $no_data_message = "No data has been recorded yet.";
}

// Close the statement
$stmt->close();

// Check if the form is submitted
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Retrieve and sanitize the form data
    $patient_id = isset($_SESSION["patient_id"]) ? $_SESSION["patient_id"] : ''; // Adjust session variable name
    $room_temp = mysqli_real_escape_string($database, $_POST["room_temp"]);
    $humidity = mysqli_real_escape_string($database, $_POST["humidity"]);
    $body_temp = mysqli_real_escape_string($database, $_POST["body_temp"]);
    $spo2 = mysqli_real_escape_string($database, $_POST["spo2"]);
    $bpm = mysqli_real_escape_string($database, $_POST["bpm"]);

    // Prepare and bind the statement
    $sql = "INSERT INTO patient_monitoring (patient_id, room_temp, humidity, body_temp, spo2, bpm, recorded_at) VALUES (?, ?, ?, ?, ?, ?, NOW())";
    $stmt = $database->prepare($sql);
    $stmt->bind_param("iddddd", $patient_id, $room_temp, $humidity, $body_temp, $spo2, $bpm);

    // Execute the statement
    if ($stmt->execute()) {
        echo "New record created successfully!";
    } else {
        echo "Error: " . $stmt->error;
    }
    $stmt->close();
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Patient Monitoring</title>
    <style>
        
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
        }
        .container {
            max-width: 600px;
            margin: 50px auto;
            padding: 20px;
            border: 1px solid #ccc;
            border-radius: 5px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
        }
        h1 {
            text-align: center;
        }
        form {
            margin-top: 20px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            font-weight: bold;
        }
        input[type="number"] {
            width: 100%;
            padding: 8px;
            margin-bottom: 10px;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
        }
        input[type="submit"] {
            width: 100%;
            background-color: #4CAF50;
            color: white;
            padding: 10px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
        }
        input[type="submit"]:hover {
            background-color: #45a049;
        }
        p {
            font-weight: bold;
        }
    
    </style>
</head>
<body>
    <div class="container">
        <h1>Patient Monitoring</h1>
        <!-- Display patient information -->
        <p>Patient Name: <?php echo $pname; ?></p>
        <!-- Add other patient details here -->

        <?php
        if (isset($no_data_message)) {
            echo $no_data_message;
        } else {
            echo "Room Temperature: " . $room_temp . "<br>";
            echo "Humidity: " . $humidity . "<br>";
            echo "Body Temperature: " . $body_temp . "<br>";
            echo "SpO2: " . $spo2 . "<br>";
            echo "BPM: " . $bpm . "<br>";
        }
        ?>

        <form method="post" action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]); ?>">
            <label for="room_temp">Room Temperature (°C):</label>
            <input type="number" step="0.01" name="room_temp" id="room_temp" required><br>
            <label for="humidity">Humidity (%):</label>
            <input type="number" step="0.01" name="humidity" id="humidity" required><br>
            <label for="body_temp">Body Temperature (°C):</label>
            <input type="number" step="0.01" name="body_temp" id="body_temp" required><br>
            <label for="spo2">SpO2 (%):</label>
            <input type="number" name="spo2" id="spo2" required><br>
            <label for="bpm">BPM:</label>
            <input type="number" name="bpm" id="bpm" required><br>
            <input type="submit" name="submit" value="Submit">
        </form>
    </div>
</body>
</html>