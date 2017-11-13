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
let GOOGLE_FILE={};
GOOGLE_FILE["Symbol"]="Symbol1";
GOOGLE_FILE["Name"]="Name1";
GOOGLE_FILE["Type"]="Type1";
GOOGLE_FILE["Date"]="Date1";
GOOGLE_FILE["Shares"]="Shares1";
GOOGLE_FILE["Price"]="Price1";
GOOGLE_FILE["Cashvalue"]="Cashvalue1";
GOOGLE_FILE["Commission"]="Commission1";
GOOGLE_FILE["Notes"]="Notes1";
console.log(Object.keys(GOOGLE_FILE));
let GOOGLE_KEYS = Object.keys(GOOGLE_FILE);
let match = checkFormat(output, GOOGLE_KEYS);
if (match)	
	console.log("successfully matched file format:GOOGLE_FILE");
else
	console.log("unable to match format:GOOGLE_FILE");
