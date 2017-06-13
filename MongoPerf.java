import com.mongodb.BasicDBObject;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.MongoClient;

public class MongoPerf {
    public static final int MAX_RECORDS = 5000;

    public static void main(String[] args) throws Exception {
        MongoClient mongoClient = new MongoClient("localhost", 27017);
        DB db = mongoClient.getDB("development");
        DBCollection coll = db.getCollection("TMP");

        for (int index = 0; index < 10000; ++index) {
            long start = System.nanoTime();
            int count = 0;
            BasicDBObject query = new BasicDBObject();
            DBCursor cursor = coll.find(query).limit(MAX_RECORDS).batchSize(MAX_RECORDS);
            long bytes = 0;
            try {
                while (cursor.hasNext()) {
                    cursor.next();
                    ++count;
                }


            } finally {
                cursor.close();
            }
            long end = System.nanoTime();
            System.out.println("time(uS) " + (0.001 * (end - start)) + " count= " + count);
        }
    }
}
