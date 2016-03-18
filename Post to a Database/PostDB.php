   <?php
/* Server 1.2
   Using the following information, establish a connection with a database, and insert the previous 

information into the Horse table:

dbName = "sonic",  dbUser = "doctorrobotnik", dbPassword = "g0ldr1ngs", server = “150.141.163.150”
    */
    if (!empty($_POST)) {
        #This program will connect to the specified database
        $servername = "150.141.163.150";
        $username = "doctorrobotnik";
        $password = "g0ldr1ngs";
        try {
            #Use PDO to protect against sql injection
            $conn = new PDO("mysql:host=$servername;dbname=sonic", $username, $password);
            // set the PDO error mode to exception
            $conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
            //echo "Connected successfully"; 
        } 
        catch(PDOException $e){
            echo "Connection failed: " . $e->getMessage();
        }
        addHorseToDataBase($conn);
    }
    function addHorseToDataBase($conn) {
        #trim the strings
        $table = trim($_POST['action']);
        if (strcmp($table,"Horse") != 0) {
            #wrong action must quit
            echo "failed on action";
            exit();
        }
        $name = trim($_POST['name']);
        $breed = trim($_POST['breed']);
        
        #Check to make sure all ints are positive
        if (($_POST['height']) > 0) {
            $height = ($_POST['height']);
        }else {
            echo "failed on Height parameter";
            exit();
        }
        if (($_POST['weight']) > 0) {
            $weight = ($_POST['weight']);
        }else {
            echo "failed on weight parameter";
            exit();
        }
        if (($_POST['age']) > 0) {
            $age = ($_POST['age']);
        }else {
            echo "failed on age parameter";
            exit();
        }
        $sql = "INSERT INTO sonic.Horse(name, breed, height, weight, age) VALUES (:name, :breed, :height, :weight, :age)";
        $query = $conn->prepare($sql);
        $query->execute(array( 
            ':name' => $name,
            ':breed' => $breed,
            ':height' => $height, 
            ':weight' => $weight,
            ':age' => $age
        ));
        $conn = null;
        echo "Success!";
    }
    ?>
