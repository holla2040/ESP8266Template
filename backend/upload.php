<?php
    if ($_SERVER['REQUEST_METHOD'] == 'POST') {
	    $headers = getallheaders();
        $target_filename = "uploads/" . $headers["Filename"];
        $fileType = strtolower(pathinfo($target_filename,PATHINFO_EXTENSION));
        if ($fileType == "csv") {
            file_put_contents($target_filename,file_get_contents('php://input'));
            return;
        }
/* <form action="upload.php" method="post" enctype="multipart/form-data">
    <input type="file" name="fileToUpload" id="fileToUpload"><br>
    <input type="submit" value="Upload Image" name="submit">
</form> */
    }
?>
<!DOCTYPE html>
<html>
<body>
<h3>Log Files</h3>
<hr>
<?php
    foreach(scandir("uploads/") as $fn) {
        if ($fn[0] != ".") {
            echo "<a href=\"uploads/$fn\">$fn</a><br>";
        }
    }
?>
</body>
</html>
