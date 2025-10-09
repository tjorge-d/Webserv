#!/usr/bin/php
<?php
$query_string = getenv('QUERY_STRING');
parse_str($query_string, $get_params);

$name = isset($get_params['name_php']) ? $get_params['name_php'] : 'Guest';
$age = isset($get_params['age_php']) ? $get_params['age_php'] : 'unknown';

echo "Content-Type: text/html\r\n\r\n";

echo "<!DOCTYPE html>\n";
echo "<html>\n";
echo "<body>\n";
echo "<h1>Hello, $name!</h1>\n";
echo "<p>You are $age years old.</p>\n";
echo "</body>\n";
echo "</html>\n";
?>