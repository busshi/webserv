<?php

$id = rand();
$file = fopen("spawned_file_" . $id, "w");

fwrite($file, "Hello, I'm file " . $id . ": I've been spawned by the spawn_file.php script");

header('Location: http://localhost:8080')

?>
