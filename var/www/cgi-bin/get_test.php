#!/usr/bin/php
<?php
$query_string = getenv('QUERY_STRING');
parse_str($query_string, $get_params);

// Get name and age from query string
$name = isset($get_params['name_php']) ? $get_params['name_php'] : 'Guest';
$age = isset($get_params['age_php']) ? $get_params['age_php'] : 'unknown';

// Get cookies from environment variable (since this is CGI)
$cookie_header = getenv('HTTP_COOKIE');
$cookies = array();
if ($cookie_header) {
    $cookie_parts = explode(';', $cookie_header);
    foreach ($cookie_parts as $cookie) {
        $cookie = trim($cookie);
        if (strpos($cookie, '=') !== false) {
            list($key, $value) = explode('=', $cookie, 2);
            $cookies[trim($key)] = trim($value);
        }
    }
}

// Determine theme from cookie, default to 'light'
$theme = isset($cookies['theme']) ? $cookies['theme'] : 'light';

if ($theme === 'dark') {
    $bg_color = "#121212";
    $text_color = "#FFFFFF";
} else {
    $bg_color = "#FFFFFF";
    $text_color = "#000000";
}

// Send response
echo "Content-Type: text/html\r\n\r\n";

echo "<!DOCTYPE html>\n";
echo "<html>\n";
echo "<head>\n";
echo "<title>Hello Page</title>\n";
echo "<style>\n";
echo "body { background-color: $bg_color; color: $text_color; font-family: Arial, sans-serif; }\n";
echo "</style>\n";
echo "</head>\n";
echo "<body>\n";
echo "<h1>Hello, $name!</h1>\n";
echo "<p>You are $age years old.</p>\n";
echo "<p>Current theme: $theme</p>\n";
echo "</body>\n";
echo "</html>\n";
?>