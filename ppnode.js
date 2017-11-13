"use strict";
var papa = require('papaparse');
var fs = require('fs');
function checkFormat(output, format)
{
	for(let i=0;i<output.meta.fields.length;++i)
	{
		let trimmed = output.meta.fields[i].replace(/\uFEFF/g, '');	
		if (!format.includes(trimmed))
		{
			console.log("key not found:"+ trimmed);
			return false;
		}	
	}
	return true;	
}
var contents = fs.readFileSync(process.argv[2]).toString();
var output = papa.parse(contents,{header: true});
let GOOGLE_FILE={
	"Symbol":"Symbol",
	"Name":"Name",
	"Symbol":"Symbol",
	"Type":"Type",
	"Date":"Date",
	"Shares":"Shares",
	"Price":"Price",
	"Cashvalue":"Cashvalue",
	"Commission":"Commission",
	"Notes":"Notes" };

	console.log(Object.values(GOOGLE_FILE));
	let GOOGLE_KEYS = Object.values(GOOGLE_FILE);
	let match = checkFormat(output, GOOGLE_KEYS);
	if (match)	
		console.log("successfully matched file format:GOOGLE_FILE");
	else
		console.log("unable to match format:GOOGLE_FILE");
