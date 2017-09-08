"use strict";
var N=8000000;
var LOOPS=50;
var now = require("performance-now")
function sum(a,b,elements)
{
	var t0 = now();
	var sum=0;
	for(var index=0;index<N;++index)
	{
		sum+= Math.sin(a[index])*Math.cos(b[index]);
	}
	var t1 = now();
	console.log("N="+N + " time(ms) =" + (t1 - t0).toFixed(3) + " sum="+sum);
}
var a = new Array(N);
var b = new Array(N);
for(var index=0;index<N;++index)
{
	a[index]=400+index;
	b[index]=500+index;
}
for(var index=0;index<LOOPS;++index)
	sum(a,b,N);
