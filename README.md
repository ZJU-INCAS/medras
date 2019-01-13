# medras
This document introduces about MEDRAS, an auction system for emergency demand response (EDR) in colocation datacenters. 

MEDRAS is developed by Jianhai Chen who comes from ZJU-INCAS lab, Zhejiang University Inteligence computing and system lab. 
The current version of MEDRAS is 1.0. The medras includes two parts. The first is a MEDRAS platform and another part is the MEDR auction service program (medrasp). 

The MEDRAS platform is realized as a web site system based on PHP/Mysql. The MEDRAS platform are responsible for managing the basic auction information. The system users involve colocation operator and tenant who participate aucitons for EDR. At first, colocation operator sends EDR signals on the platform and launches an EDR auction. Then, the datacenter tenant users can find the auction information in the platform and submit bids to the auction when they are willing to attend the EDR auction activity. The platform collects all the tenant bids and send a request for bidding decision making to the MEDRAS service program. 

The medrasp is developed by C/C++ programming language. It charges making decision of winner bids amongst a set of bid participants. It invokes decision algorithms realized based on the EDR mechanisms. The mechanism algorithms consist of dynamic programming optimal, FPTAS and VCG-based mechanism, FPTAS payment and VCG payment algorithms. The FPTAS mechanism is the best one.

All the algorithms locate in the medrasp directory. If you want to know more details about medras please see specifically in the medrasp code.

