var http = require('http');
var util = require('util');
var spwan = require('child_process').spawn;
var fs = require('fs');
http.createServer(function (req, res) {
	if (req.url.substr(0, 4) == '/st/') {
		fs.readFile(req.url.substr(1), function(err, data) {
			if (err) {
				res.writeHead(404, {'Content-Type': 'text/plain'});
				res.end('Not Found')
			} else {
				var match = req.url.substr(4).match(/\.[^.]+$/);
				var contentType = 'text/html';
				if (match == '.png') {
					contentType = 'image/png';
				}
				res.writeHead(200, {'Content-Type': contentType});
				res.write(data);
				res.end();
			}
		});
	} else {
		cmd = spwan('./t1.out', ['js', req.url.substr(1)]);
		res.writeHead(200, {'Content-Type': 'text/plain'});
		cmd.stdout.on('data', function (data) {
			res.write(data);
		});
		cmd.on('exit', function (data) {
			res.end('');
		});
	}
}).listen(1337, "127.0.0.1");
console.log('Server running at http://127.0.0.1:1337/');
