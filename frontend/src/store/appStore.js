import {defineStore} from 'pinia'
import {ref} from 'vue'


export const useAppStore =  defineStore('app', ()=>{

    const getAllInRange = async (start,end)=>{ 
// FETCH REQUEST WILL TIMEOUT AFTER 20 SECONDS   
const controller    = new AbortController(); 
const signal        = controller.signal; 
const id            = setTimeout(()=>{controller.abort()},60000);    
const URL           = `/api/climo/get/${start}/${end}`;         

try { 
    const response  = await fetch(URL,{ method: 'GET', signal: signal  }); 
    if (response.ok){ const data = await response.json(); 
        let keys= Object.keys(data); 
        if(keys.includes("status"))
            { if(data["status"] == "found" )
                { // console.log(data["data"]  )
                  return data["data"];
                } 
                if(data["status"] == "failed")
                    { console.log("getAllInRange returned no data"); }                 
}}
    else{ const data= await response.text();
        console.log(data); }   

} 
catch(err){      
console.error('getAllInRange error: ', err.message);           
}     
return []          
} 
    /*  
    The composition API way of defining a Pinia store
    ref() s become state properties
    computed() s become getters
    function() s become actions  
    */ 

    // STATES   


    // ACTIONS
    

    return { 
    // EXPORTS
    getAllInRange	
            
       }
},{ persist: true  });