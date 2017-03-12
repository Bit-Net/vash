
/* Define the number of leaves to be used in the animation */
const NUMBER_OF_LEAVES = 15;
var AdsUrl = '';
var MsgContainer = null;
var MyCanvas = null;
var BnsFrm = null;
var BnsDiv = null;
var WebViewID = 3;

function SetAdTxtAndUrl(vSender, vTxt, vUrl)
{
	AdsUrl = vUrl;
	//alert(MsgContainer.innerHTML);
	var v2 = vTxt.replace(/\|/g, "<br>");
	MsgContainer.innerHTML = '<br>' + v2 + '<br><br><br>';
	document.getElementById('BnsSayer').innerText = vSender;
	//alert(MsgContainer.innerText);
}

/* 
    Called when the "Falling Leaves" page is completely loaded.
*/
function init()
{
    /* Get a reference to the element that will contain the leaves */
    var container = document.getElementById('leafContainer');
    MsgContainer = document.getElementById('BonusMsg');
	MyCanvas = document.getElementById('MyCanvas');
	BnsFrm = document.getElementById('BnsFrm');
	BnsDiv = document.getElementById("BnsDiv");

    /* Fill the empty container with new leaves */
    for (var i = 0; i < NUMBER_OF_LEAVES; i++) 
    {
        container.appendChild(createALeaf());
    }
}


/*
    Receives the lowest and highest values of a range and
    returns a random integer that falls within that range.
*/
function randomInteger(low, high)
{
    return low + Math.floor(Math.random() * (high - low));
}


/*
   Receives the lowest and highest values of a range and
   returns a random float that falls within that range.
*/
function randomFloat(low, high)
{
    return low + Math.random() * (high - low);
}


/*
    Receives a number and returns its CSS pixel value.
*/
function pixelValue(value)
{
    return value + 'px';
}


/*
    Returns a duration value for the falling animation.
*/

function durationValue(value)
{
    return value + 's';
}


/*
    Uses an img element to create each leaf. "Leaves.css" implements two spin 
    animations for the leaves: clockwiseSpin and counterclockwiseSpinAndFlip. This
    function determines which of these spin animations should be applied to each leaf.
    
*/
function MyImgOnClick(abc)
{
    //alert(MyCanvas);
	//MyCanvas.onmousedown();
	//document.getElementById("MyCanvas").onmousedown();
	//document.getElementById("MyCanvas").onmouseup();
	//MyCanvas.slideToggle();
	abc.parentElement.removeChild(abc);
	var aTxt = '1';
	var vId = 1;
	if( AdsUrl.length ){
		BnsFrm.src = AdsUrl;
		BnsDiv.style.display = 'block';
		aTxt = '2';
		vId = 2;
	}
	container = document.getElementById('leafContainer');
    for (var i = 0; i < NUMBER_OF_LEAVES; i++) 
    {
		aDiv = container.getElementsByTagName('div')[0];
		if( aDiv )
		{
			container.removeChild(aDiv);
		}
    }
	document.getElementById('dialogDiv').style.display = 'none';
	if( WebViewID > 2 ){ BitNet3.SendChatMessage(aTxt, WebViewID, vId); }
}

function createALeaf()
{
    /* Start by creating a wrapper div, and an empty img element */
    var leafDiv = document.createElement('div');
    var image = document.createElement('img');
	image.onclick = function () {
		MyImgOnClick(this); 
		//this.parentElement.removeChild(this);
	};
    
    /* Randomly choose a leaf image and assign it to the newly created element */
    image.src = 'images/realLeaf' + randomInteger(1, 5) + '.png';
    
    leafDiv.style.top = "-80px";

    /* Position the leaf at a random location along the screen */
	var scrWidth = window.screen.width;
    leafDiv.style.left = pixelValue(randomInteger(0, scrWidth));
    
    /* Randomly choose a spin animation */
    var spinAnimationName = (Math.random() < 0.5) ? 'clockwiseSpin' : 'counterclockwiseSpinAndFlip';
    
    /* Set the -webkit-animation-name property with these values */
    leafDiv.style.webkitAnimationName = 'fade, drop';
    image.style.webkitAnimationName = spinAnimationName;
    
    /* Figure out a random duration for the fade and drop animations */
    var fadeAndDropDuration = durationValue(randomFloat(5, 11));
    
    /* Figure out another random duration for the spin animation */
    var spinDuration = durationValue(randomFloat(4, 8));
    /* Set the -webkit-animation-duration property with these values */
    leafDiv.style.webkitAnimationDuration = fadeAndDropDuration + ', ' + fadeAndDropDuration;

    var leafDelay = durationValue(randomFloat(0, 5));
    leafDiv.style.webkitAnimationDelay = leafDelay + ', ' + leafDelay;

    image.style.webkitAnimationDuration = spinDuration;

    // add the <img> to the <div>
    leafDiv.appendChild(image);

    /* Return this img element so it can be added to the document */
    return leafDiv;
}


/* Calls the init function when the "Falling Leaves" page is full loaded */
window.addEventListener('load', init, false);


	$(function(){
var Fireworks = function(){
var self = this;
var rand = function(rMi, rMa){return ~~((Math.random()*(rMa-rMi+1))+rMi);}
var hitTest = function(x1, y1, w1, h1, x2, y2, w2, h2){return !(x1 + w1 < x2 || x2 + w2 < x1 || y1 + h1 < y2 || y2 + h2 < y1);};
window.requestAnimFrame=function(){return window.requestAnimationFrame||window.webkitRequestAnimationFrame||window.mozRequestAnimationFrame||window.oRequestAnimationFrame||window.msRequestAnimationFrame||function(a){window.setTimeout(a,1E3/60)}}();

self.init = function(){ 
self.canvas = document.createElement('canvas');             
self.canvas.id = "MyCanvas";
//self.leaf = document.createElement('div');             
//self.leaf.id = "leafContainer";
//self.canvas.appendChild(self.leaf);
self.leaf = document.getElementById("leafContainer");
self.leaf.appendChild(self.canvas);
self.canvas.width = self.cw = $(window).innerWidth();
self.canvas.height = self.ch = $(window).innerHeight();         
self.particles = [];    
self.partCount = 150;
self.fireworks = [];    
self.mx = self.cw/2;
self.my = self.ch/2;
self.currentHue = 30;
self.partSpeed = 5;
self.partSpeedVariance = 10;
self.partWind = 50;
self.partFriction = 5;
self.partGravity = 1;
self.hueMin = 0;
self.hueMax = 360;
self.fworkSpeed = 6;
self.fworkAccel = 10;
self.hueVariance = 30;
self.flickerDensity = 25;
self.showShockwave = true;
self.showTarget = true;
self.clearAlpha = 25;

//$(document.body).append(self.canvas);
//self.container2 = document.getElementById("container");
//self.container2.appendChild(self.canvas);
self.ctx = self.canvas.getContext('2d');
self.ctx.lineCap = 'round';
self.ctx.lineJoin = 'round';
self.lineWidth = 4;
self.bindEvents();          
self.canvasLoop();

self.canvas.onselectstart = function() {
return false;
};
};      

self.createParticles = function(x,y, hue){
var countdown = self.partCount;
while(countdown--){
var newParticle = {
    x: x,
    y: y,
    coordLast: [
        {x: x, y: y},
        {x: x, y: y},
        {x: x, y: y}
    ],
    angle: rand(0, 360),
    speed: rand(((self.partSpeed - self.partSpeedVariance) <= 0) ? 1 : self.partSpeed - self.partSpeedVariance, (self.partSpeed + self.partSpeedVariance)),
    friction: 1 - self.partFriction/100,
    gravity: self.partGravity/2,
    hue: rand(hue-self.hueVariance, hue+self.hueVariance),
    brightness: rand(50, 80),
    alpha: rand(40,100)/100,
    decay: rand(10, 50)/1000,
    wind: (rand(0, self.partWind) - (self.partWind/2))/25,
    lineWidth: self.lineWidth
};              
self.particles.push(newParticle);
}
};


self.updateParticles = function(){
var i = self.particles.length;
while(i--){
var p = self.particles[i];
var radians = p.angle * Math.PI / 180;
var vx = Math.cos(radians) * p.speed;
var vy = Math.sin(radians) * p.speed;
p.speed *= p.friction;
                
p.coordLast[2].x = p.coordLast[1].x;
p.coordLast[2].y = p.coordLast[1].y;
p.coordLast[1].x = p.coordLast[0].x;
p.coordLast[1].y = p.coordLast[0].y;
p.coordLast[0].x = p.x;
p.coordLast[0].y = p.y;

p.x += vx;
p.y += vy;
p.y += p.gravity;

p.angle += p.wind;              
p.alpha -= p.decay;

if(!hitTest(0,0,self.cw,self.ch,p.x-p.radius, p.y-p.radius, p.radius*2, p.radius*2) || p.alpha < .05){                  
    self.particles.splice(i, 1);    
}
};
};

self.drawParticles = function(){
var i = self.particles.length;
while(i--){
var p = self.particles[i];                          

var coordRand = (rand(1,3)-1);
self.ctx.beginPath();                               
self.ctx.moveTo(Math.round(p.coordLast[coordRand].x), Math.round(p.coordLast[coordRand].y));
self.ctx.lineTo(Math.round(p.x), Math.round(p.y));
self.ctx.closePath();               
self.ctx.strokeStyle = 'hsla('+p.hue+', 100%, '+p.brightness+'%, '+p.alpha+')';
self.ctx.stroke();              

if(self.flickerDensity > 0){
    var inverseDensity = 50 - self.flickerDensity;                  
    if(rand(0, inverseDensity) === inverseDensity){
        self.ctx.beginPath();
        self.ctx.arc(Math.round(p.x), Math.round(p.y), rand(p.lineWidth,p.lineWidth+3)/2, 0, Math.PI*2, false)
        self.ctx.closePath();
        var randAlpha = rand(50,100)/100;
        self.ctx.fillStyle = 'hsla('+p.hue+', 100%, '+p.brightness+'%, '+randAlpha+')';
        self.ctx.fill();
    }   
}
};
};


self.createFireworks = function(startX, startY, targetX, targetY){
var newFirework = {
x: startX,
y: startY,
startX: startX,
startY: startY,
hitX: false,
hitY: false,
coordLast: [
    {x: startX, y: startY},
    {x: startX, y: startY},
    {x: startX, y: startY}
],
targetX: targetX,
targetY: targetY,
speed: self.fworkSpeed,
angle: Math.atan2(targetY - startY, targetX - startX),
shockwaveAngle: Math.atan2(targetY - startY, targetX - startX)+(90*(Math.PI/180)),
acceleration: self.fworkAccel/100,
hue: self.currentHue,
brightness: rand(50, 80),
alpha: rand(50,100)/100,
lineWidth: self.lineWidth
};          
self.fireworks.push(newFirework);

};


self.updateFireworks = function(){
var i = self.fireworks.length;

while(i--){
var f = self.fireworks[i];
self.ctx.lineWidth = f.lineWidth;

vx = Math.cos(f.angle) * f.speed,
vy = Math.sin(f.angle) * f.speed;
f.speed *= 1 + f.acceleration;              
f.coordLast[2].x = f.coordLast[1].x;
f.coordLast[2].y = f.coordLast[1].y;
f.coordLast[1].x = f.coordLast[0].x;
f.coordLast[1].y = f.coordLast[0].y;
f.coordLast[0].x = f.x;
f.coordLast[0].y = f.y;

if(f.startX >= f.targetX){
    if(f.x + vx <= f.targetX){
        f.x = f.targetX;
        f.hitX = true;
    } else {
        f.x += vx;
    }
} else {
    if(f.x + vx >= f.targetX){
        f.x = f.targetX;
        f.hitX = true;
    } else {
        f.x += vx;
    }
}

if(f.startY >= f.targetY){
    if(f.y + vy <= f.targetY){
        f.y = f.targetY;
        f.hitY = true;
    } else {
        f.y += vy;
    }
} else {
    if(f.y + vy >= f.targetY){
        f.y = f.targetY;
        f.hitY = true;
    } else {
        f.y += vy;
    }
}               

if(f.hitX && f.hitY){
    self.createParticles(f.targetX, f.targetY, f.hue);
    self.fireworks.splice(i, 1);
    
}
};
};

self.drawFireworks = function(){
var i = self.fireworks.length;
self.ctx.globalCompositeOperation = 'lighter';
while(i--){
var f = self.fireworks[i];      
self.ctx.lineWidth = f.lineWidth;

var coordRand = (rand(1,3)-1);                  
self.ctx.beginPath();                           
self.ctx.moveTo(Math.round(f.coordLast[coordRand].x), Math.round(f.coordLast[coordRand].y));
self.ctx.lineTo(Math.round(f.x), Math.round(f.y));
self.ctx.closePath();
self.ctx.strokeStyle = 'hsla('+f.hue+', 100%, '+f.brightness+'%, '+f.alpha+')';
self.ctx.stroke();  

if(self.showTarget){
    self.ctx.save();
    self.ctx.beginPath();
    self.ctx.arc(Math.round(f.targetX), Math.round(f.targetY), rand(1,8), 0, Math.PI*2, false)
    self.ctx.closePath();
    self.ctx.lineWidth = 1;
    self.ctx.stroke();
    self.ctx.restore();
}
    
if(self.showShockwave){
    self.ctx.save();
    self.ctx.translate(Math.round(f.x), Math.round(f.y));
    self.ctx.rotate(f.shockwaveAngle);
    self.ctx.beginPath();
    self.ctx.arc(0, 0, 1*(f.speed/5), 0, Math.PI, true);
    self.ctx.strokeStyle = 'hsla('+f.hue+', 100%, '+f.brightness+'%, '+rand(25, 60)/100+')';
    self.ctx.lineWidth = f.lineWidth;
    self.ctx.stroke();
    self.ctx.restore();
}
};
};

self.bindEvents = function(){
$(window).on('resize', function(){          
clearTimeout(self.timeout);
self.timeout = setTimeout(function() {
    self.canvas.width = self.cw = $(window).innerWidth();
    self.canvas.height = self.ch = $(window).innerHeight();
    self.ctx.lineCap = 'round';
    self.ctx.lineJoin = 'round';
}, 100);
});

$(self.canvas).on('mousedown', function(e){
self.mx = e.pageX - self.canvas.offsetLeft;
self.my = e.pageY - self.canvas.offsetTop;
self.currentHue = rand(self.hueMin, self.hueMax);
self.createFireworks(self.cw/2, self.ch, self.mx, self.my); 
document.getElementById('audiotag1').src = "";
document.getElementById('audiotag1').src = "bong.wav";
document.getElementById('audiotag1').play();


$(self.canvas).on('mousemove.fireworks', function(e){
    self.mx = e.pageX - self.canvas.offsetLeft;
    self.my = e.pageY - self.canvas.offsetTop;
    self.currentHue = rand(self.hueMin, self.hueMax);
    self.createFireworks(self.cw/2, self.ch, self.mx, self.my);                                 
});             
});

$(self.canvas).on('mouseup', function(e){
$(self.canvas).off('mousemove.fireworks');                                  
});
        
}

self.clear = function(){
self.particles = [];
self.fireworks = [];
self.ctx.clearRect(0, 0, self.cw, self.ch);
};


self.canvasLoop = function(){
requestAnimFrame(self.canvasLoop, self.canvas);         
self.ctx.globalCompositeOperation = 'destination-out';
self.ctx.fillStyle = 'rgba(0,0,0,'+self.clearAlpha/100+')';
self.ctx.fillRect(0,0,self.cw,self.ch);
self.updateFireworks();
self.updateParticles();
self.drawFireworks();           
self.drawParticles();

};

self.init();        

}
var fworks = new Fireworks();

});