<?php

header("Content-Type: application/json");

$data_file_path = dirname(__FILE__) . "/data.json";

function save_data_to_json($data) {
    global $data_file_path;
    file_put_contents($data_file_path, json_encode(array_values($data)));
}

function read_data_from_json() {
    global $data_file_path;
    if (!file_exists($data_file_path)) {
        save_data_to_json([]);
    }
	return json_decode(file_get_contents($data_file_path));
}

$json_data = read_data_from_json();
switch ($_SERVER["REQUEST_METHOD"]) {
    case "GET":
        http_response_code(200);
        echo json_encode($json_data);
        break;

    case "POST":
        $name = file_get_contents("php://input");
	 	if ($name === "") 
		{
			http_response_code(400);
			echo json_encode(["error" => "Name cannot be empty"]);
		 	break; 
		}
		if (in_array($name, $json_data, true))
		{
			http_response_code(409);
			echo json_encode(["error" => "Duplicate name"]);
			break;
		}
		$json_data[] = $name;
        save_data_to_json($json_data);
        http_response_code(201);
        echo json_encode($json_data);
        break;

    case "DELETE":
		$name_to_delete = $_GET["name"] ?? "";
		if (!in_array($name_to_delete, $json_data, true)) 
		{ 
			http_response_code(404); 
			echo json_encode(["error" => "Name not found"]); 
			break;
		} 
		$json_data = array_filter( $json_data, function ($name) use ($name_to_delete) { return $name !== $name_to_delete; } ); 
		save_data_to_json($json_data); 
		http_response_code(204);
		break; 
		
	default:
		http_response_code(405); 
		header("Allow: GET, POST, DELETE"); 
		break;
}
?>
