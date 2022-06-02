# ΣυσΠρο Εργασία 2

## Compile & Run
Για την δημιουργία εκτελέσιμου πρέπει να τρέξετε την εντολή: make all
Για να ανοίξει η εφαρμογή δέχεται τις παραμέτρους που περιγράφονται στην εκφώνηση δηλαδή:
./travelMonitorClient –m numMonitors -b socketBufferSize -c cyclicBufferSize -s
sizeOfBloom -i input_dir -t numThreads
όπου οι παράμετροι δίνονται με οποιαδήποτε σειρά.
Το bufferSize πρέπει να έχει μέγεθος αρκετό ώστε να μπορούν να  μεταφερθούν δεδομένα όπως
το σύνολο των χωρών που συμμετέχουν σε κάθε Monitor και εντολές από το travelMonitor στο Monitor
χωρίς να σπάνε έχοντας υποθέσει ότι αυτά τα δεδομένα είναι μικρού μεγέθους.

## Εφαρμογή
***Αρχικοποίηση***
Αρχικά τρέχει το travelMonitorClient όπου δημιουργεί ένα socket για κάθε monitorServer που θα υπάρχει
στην εφαρμογή.Για τη δημιουργία του socket βρίσκει την IP του μηχανήματος ενώ για port χρησιμοποιεί ports
από το 49152 τα οποία είναι ελύθερα.Στη συνέχεια βρίσκει τα paths των φακέλων που θα μεταβιβάσει στα monitorServers
και κάνει fork numMonitors.Κάθε monitorServer αφού εξάγει τα paths δημιουργουργεί numThreads Threads περνώντας τους ως
παραμέτρους δείκτες στις κοινόχρηστες δόμες και το bloomSize.Αφού δημιουργηθούν τα Threads γίνεται αρχικοποίηση των δομών
με τη χρήση του cyclic buffer.Για την αρχικοποίηση αυτή χρησιμοποιούνται ένας mutex που συσχετίζεται με δύο cond vars τα
οποία υποδηλώνουν αν το  cyclic buffer είναι γεμάτο ή άδειο,επίσης χρησιμοποιείται μια global var η οποία δείχνει το σύνολο
των χωρών όπου απομένουν να αρχικοποιηθούν.Το cyclic buffer είναι υλοποιημένο σαν linked list με μέγιστο μέγεθος cyclicBufferSize.
Κάθε Thread αρχικοποιεί το πρώτο path που βρίσκει στο cyclic buffer και το διαγράφει από τη λίστα.Η εισαγωγή δεδομένων στο  cyclic buffer
από το monitorServer γίνεται με μεγαλύτερη προτεραιότητα από την αφαίρεση απο τα threads.Αφού γίνει η αρχικοποίηση των δομών τα threads 
κλείνουν,το  monitorServer βρίσκει την IP του μηχανήματος, δημιουργεί socket, κάνει bind + listen χρησιμοποιώντας την IP και το port
που πήρε από το travelMonitorClient και συνδεέται με το travelMonitorClient.Τέλος μεταφέρονται τα δεδομένα στο travelMonitorClient όπως 
στην εργασία 2 με την επικοινωνία να γίνεται μέσω των sockets.

***/travelRequest***
H λειτουργία είναι ακριβώς ίδια με τη 2η άσκηση. Η επικοινωνία γίνεται πάνω από socket.

***/travelStats***
H λειτουργία είναι ακριβώς ίδια με τη 2η άσκηση.

***/addVaccinationRecords***
Το travelMonitorClient στέλνει στο κατάλληλο monitorServer την εντολή /addVaccinationRecords μέσω socket.Το monitorServer ξαναδημιουργεί
numThreads Threads,καταστρέφει τα δεδομένα που έχει εκτός από τις χώρες που ελέγχει και τα ξανά αρχικοποιεί με την χρήση του cyclic buffer.
Το travelMonitorClient έχει επίσης καταστρέψει τα δεδομένα που σχετίζοντα με το monitorServer εκτός από τις χώρες.Αφού το monitorServer τελειώσει 
την αρχικοποίηση γράφει τα δεδομένα στο travelMonitorClient όπως και την πρώτη φορά εκτός από τις χώρες.

***/searchVaccinationStatus***
H λειτουργία είναι ακριβώς ίδια με τη 2η άσκηση. Η επικοινωνία γίνεται πάνω από socket.

***/exit***
Το travelMonitorClient στέλνει στα monitorServers την εντολή /exit μέσω sockets και τα περιμένει να τερματίσουν.Κάθε monitorServer τυπώνει το log_file
του, ελευθερώνει τις δομές, κλείνει τις συνδέσεις και τερματίζει.Αφού τερματίσουν τα monitorServers το travelMonitorClient τυπώνει το log_file του, 
ελευθερώνει τις δομές, κλείνει τις συνδέσεις και τερματίζει επίσης.
