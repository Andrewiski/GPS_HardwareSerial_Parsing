var http = require('http');
var fs = require('fs');
var path = require('path');
const querystring = require('querystring');

var handleFileRequest = function(request, response, filePath){
	
	switch(filePath){
		case "/":
			filePath = './sdCardFiles/webserver/index.htm';
			break;
		case "/favicon.ico":
			filePath = './sdCardFiles/webserver/favicon.ico';
			break;
		case "/sample.json":
		case "/getGpsData":
			filePath = './nodeServerTestFiles/sampleLog.json';
			break;
		case "/autoconnectMenu":
			filePath = './nodeServerTestFiles/autoConnectMenu.htm';
			break;
		case "/getCurrentPosition":
			filePath = './nodeServerTestFiles/getCurrentPosition.json';
			break;
		case "/getCurrentPath":
			filePath = './nodeServerTestFiles/getCurrentPath.json';
			break;
		default:
			filePath = './sdCardFiles' + filePath;
			break;
	}
	
    var extname = path.extname(filePath);
    var contentType = 'text/html';
    switch (extname) {
        case '.js':
            contentType = 'text/javascript';
            break;
        case '.css':
            contentType = 'text/css';
            break;
        case '.json':
            contentType = 'application/json';
            break;
        case '.png':
            contentType = 'image/png';
            break;      
        case '.jpg':
            contentType = 'image/jpg';
            break;
        case '.wav':
            contentType = 'audio/wav';
            break;
		case ".ico":
    		contentType = "image/x-icon";
			break;
		case ".zip":
    		contentType ="application/x-zip";
			break;
		case ".gz":
    		contentType ="application/x-gzip";
			break;
			
    }
	 
	if(fs.existsSync(filePath)=== false && fs.existsSync(filePath + ".gz")=== true){
		filePath = filePath+'.gz';
	}
	console.log('filePath ' + filePath + ' ...');
	
	
	
    fs.readFile(filePath, function(error, content) {
        if (error) {
			console.log('Error ', error)
            if(error.code == 'ENOENT'){
				response.writeHead(404, { 'Content-Type': 'text/plain' });
                response.end('Sorry, check with the site admin for error: '+error.code+' ..\n');
            }
            else {
				response.writeHead(500, { 'Content-Type': 'text/plain' });
                response.end('Sorry, check with the site admin for error: '+error.code+' ..\n');
            }
        }
        else {
            response.writeHead(200, { 'Content-Type': contentType });
            response.end(content, 'utf-8');
        }
    });
}

var excludedFileNames = ['.gitattributes', '.gitignore'];
var excludedFolders = ['System Volume Information'];

http.createServer(function (request, response) {
    console.log('request starting ' + request.url + ' ...');
	try{
	    var filePath = request.url;
		//strip the queryString
		var qsPosition = filePath.indexOf("?");
		var qs = null;	
		if(qsPosition > 0){
			var queryStr = filePath.substring(qsPosition+1);
			qs = querystring.parse(filePath.substring(qsPosition+1));
			console.log("QueryString: " + queryStr);
			filePath = filePath.substring(0,qsPosition);
			
			
			
		}
		
		
		switch(filePath){
			case "/list":
				var folderpath = "./sdCardFiles";
				if(qs && qs.dir){
					folderpath = folderpath + qs.dir;
				}
				response.writeHead(200, { 'Content-Type': 'application/json' });
				response.write("[", 'utf-8');
				var files = fs.readdirSync(folderpath);
			    // listing all files using forEach
				var addComma = '';
			    files.forEach(function (file) {
			        // Do whatever you want to do with the file
			        // add local file
			        if (fs.lstatSync(path.join(folderpath, file)).isFile()) {
			            if (excludedFileNames.includes(file) === false) {
			                response.write(addComma + '{"dev":"sd", "type":"file","name":"' + file + '"}', 'utf-8');
			            }
			            //writeToLog("added file to zip " + path.join(folderpath, file) );
			        } else {
			            if (fs.lstatSync(path.join(folderpath, file)).isDirectory()) {
			                if (excludedFolders.includes(file) === false) {
			                 	response.write(addComma + '{"dev":"sd", "type":"dir","name":"' + file + '"}', 'utf-8');   
			                }
			            }
			        }
					if (addComma === ''){
						addComma = ',';
					}
			    });
	            response.end("]", 'utf-8');
				break;
			case "/getCurrentPosition":
				response.writeHead(200, { 'Content-Type': 'application/json' });
				response.end('{ "valid": true, "lat": 42.091518, "lng": -85.584163, "date": "2020-05-12T19:21:26.000Z", "ang": 182 }', 'utf-8');
				break;
			
			default:
				handleFileRequest(request, response, filePath);
		}
	
	
    
	}catch(ex){
		console.log('Error ', ex)
	}

}).listen(8125);
console.log('Server running at http://127.0.0.1:8125/');