url = "192.168.1.148"

function ping() {
    ping = new XMLHttpRequest();

    ping.timeout = 5000;
    ping.onreadystatechange = function(){

    
        if(ping.readyState == 4){
            if(ping.status == 200){
                //result = ping.getAllResponseHeaders();
                //document.body.innerHTML += "</br>" + result + "</br>";
                document.location.href=url;
            }
        }

    }
    ping.ontimeout = function(){
        console.log("TimeOut");
        ping()
    }
    ping.open("GET", url, true);    
    ping.send();


}
ping();
