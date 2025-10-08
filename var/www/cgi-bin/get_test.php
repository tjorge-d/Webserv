#!/usr/bin/php
<?php
$name = isset($_GET['name']) ? $_GET['name'] : 'Guest';
$age = isset($_GET['age']) ? $_GET['age'] : 'unknown';

header('Content-Type: text/html');

echo "<!DOCTYPE html>\n";
echo "<html>\n";
echo "<body>\n";
echo "<h1>Hello, $name!</h1>\n";
echo "<p>You are $age years old.</p>\n";
echo "</body>\n";
echo "</html>\n";
?>