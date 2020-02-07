var model = require("./data.model")
var mqtt = require('mqtt')
var client = mqtt.connect('mqtt://broker.mqtt-dashboard.com')

var express = require('express')
 const socket = require("socket.io")
 const cors = require('cors');
 const bodyParser = require('body-parser');



const app =express();
app.use(cors({origin:'*'}));
app.use(bodyParser);
let x =true;

let data ;
 const server = app.listen(4000,() => {
     console.log(`started`)
 });


client.on('connect', ()=>{
    client.subscribe("/home/+");
})

client.on('message', (topic, message)=>{

    message = message.toString()
   console.log(message)
   data = message.toString()

    mSplit =  message.split("/")
    console.log(mSplit)
     jsonObj ={
        //id  :Number,
	"Gas" :mSplit[0],
    	"AC"  :mSplit[1],
        "Fan" :mSplit[2],
     "Parking":mSplit[3]
      }

    if (topic == '/home/live'){
          model.liveSchema.find({id:1},function(err, doc) {
                if (doc)
                {
                        if(doc.length < 1){
                             model.liveSchema(jsonObj).save();
                            console.log("1sec inserted")
                        }
                        else 
                        {
                            model.liveSchema.findOneAndUpdate({"id":1},jsonObj)
                            console.log("1 sec updated")
                        }
                }
            })
    }

    else if(topic == '/home/log'){
         model.logSchema(jsonObj).save() 
         console.log("----------------------------------------------1 min" )
     } 
     
})


 const io = socket(server);

 io.sockets.on('connection',(socket) => {
     console.log(`new connection id: ${socket.id}`);
     sendData(socket);
 })

 function sendData(socket){
     socket.emit(`data1`,jsonObj.Gas);
     socket.emit(`data2`,jsonObj.AC);
     socket.emit(`data3`,jsonObj.Fan);
     socket.emit(`data4`,jsonObj.Parking);

     console.log(` ${x}`);
     setTimeout(() =>{
         sendData(socket);
     },20000);
 }



//////////////////////////////////////////////////////////////////////////////
///////data.js



var mongoose = require('mongoose');
mongoose.Promise = global.Promise;

mongoose.connect(
	'mongodb://127.0.0.1/DATASTORE',function(err){
		if(err)
		console.log(err);
		else{
			console.log("Database connected");	
		}
	}
)
const Schema = mongoose.Schema;
const logDataSchema = new Schema ({
	id  :Number,
	Gas :Number,
    AC  :Number,
    Fan :Number,
    Parking:Number
    })

const liveDataSchema = new Schema ({
	id  :Number,
	Gas :Number,
    	AC  :Number,
        Fan :Number,
     Parking:Number
})

const logSchema = mongoose.model("logSchema",logDataSchema) 
const liveSchema = mongoose.model("livSchema",liveDataSchema) 

var my_schema ={
	logSchema : logSchema,
	liveSchema : liveSchema
}

module.exports = my_schema;


