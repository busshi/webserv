<?php

$contents = scandir('.');

foreach ($contents as $file) {
    if (strncmp($file, "spawned_", 8) == 0) {
        unlink($file);
    }
}

header('Location: http://localhost:8080');

?>
