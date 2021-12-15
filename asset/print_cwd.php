<?php

echo '<h1 style="text-align:center">Directory listing using only php</h1><br/>';

$contents = scandir('.');

foreach($contents as $file) {
    if ($file == '.' || $file == '..') continue ;
    echo '<a href="' . $file . '">' . $file . '</a><hr />';
}

?>
