# ΣυσΠρο Εργασία 2

## Compile & Run
Για την δημιουργία εκτελέσιμου πρέπει να τρέξετε την εντολή: make all
Για να ανοίξει η εφαρμογή δέχεται τις παραμέτρους που περιγράφονται στην εκφώνηση δηλαδή:
./travelMonitor –m numMonitors -b bufferSize -s sizeOfBloom -i input_dir
όπου οι παράμετροι δίνονται με οποιαδήποτε σειρά.
Το bufferSize πρέπει να έχει μέγεθος αρκετό ώστε να μπορούν να  μεταφερθούν δεδομένα όπως
το σύνολο των χωρών που συμμετέχουν σε κάθε Monitor και εντολές από το travelMonitor στο Monitor
χωρίς να σπάνε έχοντας υποθέσει ότι αυτά τα δεδομένα είναι μικρού μεγέθους.
Το bash script τρέχει με τον τρόπο που περιγράφεται στην εκφώνηση.

## Εφαρμογή
***Αρχικοποίηση***
Αρχικά τρέχει το travelMonitor, δέχεται τις παραμέτρους και δημιουργεί 2 named pipes για κάθε 
Monitor που θα κάνει fork.Αμέσως μετά δημιουργεί Monitors όπου κάνουν exec με παραμέτρους τα 
ονόματα των named pipes που αντιστοιχούν στο καθένα.Το κάθε Monitor διαβάζει μέσω named pipe
τα bufferSize,sizeOfBloom και input_dir ενώ στη συνέχεια διαβάζει τις χώρες που του αντιστοιχούν
και έχουν διαμοιραστεί από το travelMonitor με round robin.Το Monitor ξέρει ότι έχει διαβάσει όλες
τις χώρες όταν διαβάσει "end".Τα Monitor στη συνέχεια αφού αρχικοποιήσουν τους φακέλους των χωρών
που έλαβαν χρησιμοποιώντας τις δόμες της 1ης άσκησης στέλνουν δεδομένα στο travelMonitor.Στέλνουν
αρχικά τα ονόματα των ιών που έχουν αποθηκεύσει ακολουθούμενα από το "end".Το travelMonitor για να
δεχθεί τα δεδομένα χωρίς να μπλοκάρει περιμένοντας αργό Monitor χρησιμοποιεί την select.Αφού στείλουν
τους ιούς τα Monitor μπλοκάρουν μέχρι να τελειώσει το travelMonitor το διάβαζμα τους.Αφού ξεμπλοκάρουν
κάθε Monitor στέλνει τα blooms κάθε ιού με τη σειρά που έστειλε και τους ιούς.Η εφαρμογή διαχειρίζεται
και blooms με μέγεθος μεγαλύτερο του buffer ενώ χρησιμοποιείται ξανά η select από το travelMonitor για
αποδοτικότητα.Τέλος με τον ίδιο τρόπο τα Monitors στέλνουν τις χώρες που διαχειρίζονται και ολοκληρώνεται
η αρχικοποίηση.
***/travelRequest***
Το travelMonitor ελέγχει το κατάλληλο bloom αν επιστραφεί αρνητικό αποτέλεσμα το εκτυπώνει αλλιώς αν επιστραφεί
θετικό αποτέλεσμα επικοινωνεί με το κατάλληλο Monitor.Για να καταλάβει το Monitor ποιά εντολή πρέπει να εκτελέσει
το travelMonitor γράφει αρχικά στο pipe την εντολή, δηλαδή /travelRequest και στην συνέχεια τα δεδομένα.Το Monitor
ελέγχει τα δεδομένα και απαντάει γράφοντας αν ο citizen έχει εμβολιαστεί και την ημερομηνία εμβολιασμού.Αν το αίτημα
εγκρίθηκε ή απορρίφθηκε συσχετίζεται στο travelMonitor με τη χώρα countryTo ενώ στο Monitor με τη χώρα countryFrom 
καθώς το Monitor μπορεί να μην διαχειρίζεται την countryTo.
***/travelStats***
Δεν χρειάζεται επικοινωνία με το Monitor απλά ελέγχει τις δομές του travelMonitor και επιστραφεί το αποτέλεσμα.
***/addVaccinationRecords***
Το travelMonitor στέλνει σήμα SIGUSER1 στο κατάλληλο Monitor.Το Monitor όταν λάβει το σήμα τροποποιεί μια global variable
την οποία ελέγχει περιοδικά για αλλαγές.Αφού η global variable έχει αλλάξει μετά από τον signal handler καταστρέφει τα δεδομένα
που έχει εκτός από τις χώρες που ελέγχει και τα ξανά αρχικοποιεί.Το travelMonitor έχει επίσης καταστρέψει τα δεδομένα που σχετίζοντα
με το Monitor εκτός από τις χώρες.Αφού το Monitor τελειώσει την αρχικοποίηση γράφει τα δεδομένα στο travelMonitor όπως και την πρώτη
φορά εκτός από τις χώρες.
***/searchVaccinationStatus***
Το travelMonitor γράφει σε όλα τα Monitors την εντολέ και το ID.Τα Monitors ελέγχουν τις δομές δεδομένων και απαντάνε στο travelMonitor.
Όσα Monitors δεν διαχειρίζονται τον citizen με το κατάλληλο ID επιστρέφουν στο travelMonitor "end" ενώ αυτό που τον διαχειρίζεται επιστραφεί
τα δεδομένα και "end" όταν αυτά τελειώσουν.
***/exit***
Το travelMonitor θέτει τον handler για το SIGCHL στο default, στέλνει SIGKILL στα Monitors και τα περιμένει να τερματίσουν.
Αφού τερματίσουν εκτυπώνει το log_file, καταστρέφει τις δομές του και τερματίζει.
***Monitor SIGINT ή SIGQUIT***
Όταν το Monitor λάβει κάποιο από αυτά τα σήματα κάλει έναν handler ο οποίος τροποποιεί μια  global variable που σηματοδοτεί ότι η διεργασία
πρέπει να τελειώσει.Την global variable ελέγχει περιοδικά για αλλαγές.Αν εκτελεί μια εντολή τελειώνει πρώτα την εκτέλεση και στη συνέχεια τερματίζει
εκτυπώνοντας το log_file και καταστρέφοντας τις δομές του.Το travelMonitor ελέγχει αντίστοιχα μια global variable για όταν λάβει SIGCHL σήμα.
Επίσης στον handler του SIGCHL κάνει wait το Monitor να τερματίσει και αποθηκεύει το pid του Monitor που τερμάτισε.Όταν η global variable τροποποιηθεί
κάνει fork ένα νέο Monitor στο οποίο, αφού κάνει exec, μεταφέρει τα sizeOfBloom,bufferSize και input_dir.Στη συνέχεια του στέλνει τις χώρες που διαχειριζόταν
το παλιό  Monitor για να κάνει αρχικοποίηση, αλλά στέλνει και το string "new" πριν το "end".Έτσι το νέο Monitor αφού αρχικοποιεί τα δεδομένα καταλαβαίνει
ότι δεν πρέπει να τα στείλει στο travelMonitor αφού τα έχει ήδη.
***travelMonitor SIGINT ή SIGQUIT***
Διαχειρίζεται το σήμα όπως το Monitor.Αν εκτελεί εντολή τελειώνει την επεξεργασία της τρέχουσας εντολής και στη συνέχεια κάνει ενέργειες αντίστοιχες με την /exit.