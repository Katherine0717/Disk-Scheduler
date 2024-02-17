## Project Purpose
This is an self-studied academic project for learning operating system. 

Added a run.sh to run the project (need to specify the test cases).

#### Disk queue: 
- contains all the requests' tracks
- no duplicate requests (one track at a time)

#### Requests (Synchronous == threads):
- add one request at a time
- Initialized to track 0
- Finishes when all requests in the file have been serviced
- Call `print_request(requester, track)` when issues a track; available to be serviced ==> add to the queue.

#### Service: 
- pick a request from the queue to handle
- only start to pick when disk reaches max number of requests == `max_disk_queue`
- shortest seek time first (SSTF)
- Call `print_service(requester, track)` when servicing a track ==> remove the request from the queue.

