package br.ufscar.dc.gsdr.mfog.structs;

import com.esotericsoftware.kryo.Kryo;
import com.esotericsoftware.kryo.Serializer;
import com.esotericsoftware.kryo.io.Input;
import com.esotericsoftware.kryo.io.Output;
import org.apache.commons.lang3.SerializationUtils;
import org.json.JSONObject;

import java.io.*;
import java.util.Objects;

public class Cluster extends Serializer<Cluster> implements Serializable, SelfDataStreamSerializable<Cluster> {
    public static String CATEGORY_NORMAL = "normal";
    public static String CATEGORY_EXTENSION = "extension";
    public static String CATEGORY_NOVELTY = "novelty";
    public static String CATEGORY_NOVELTY_EXTENSION = "novelty extension";

    public static String CSV_HEADER = "id,label,category,matches,time,variance,center";

    // size,lblClasse,category,time,meanDistance,radius,center
    public long id;
    public Point center;
    public float variance;
    public String label;
    public String category;
    public long matches;
    public long time;

    public Cluster() {
    }

    private Cluster(long id, Point center, float variance, String label, String category, long matches, long time) {
        this.id = id;
        this.center = center;
        this.variance = variance;
        this.label = label;
        this.category = category;
        this.matches = matches;
        this.time = time;
    }

    public static Cluster fromJson(String src) {
        return fromJson(new JSONObject(src));
    }

    public static Cluster fromJson(JSONObject json) {
        long id = json.getLong("id");
        float variance = json.getFloat("variance");
        String label = json.getString("label");
        String category = json.getString("category");
        long matches = json.getLong("matches");
        long time = json.getLong("time");
        JSONObject centerSrc = json.getJSONObject("center");
        Point center = Point.fromJson(centerSrc);
        return new Cluster(id, center, variance, label, category, matches, time);
    }

    public static Cluster fromBytes(byte[] bytes) {
        return SerializationUtils.deserialize(bytes);
    }

    public static Cluster fromBytes(InputStream stream) {
        return SerializationUtils.deserialize(stream);
    }

    public static Cluster fromMinasCsv(String line) {
        String[] metaAndCenter = line.split(",\\[");
        String meta = metaAndCenter[0];
        String centerString = metaAndCenter[1].replaceAll("[ \\]]", "");
        String[] split = meta.split(",");
        //
        long id = Long.parseLong(split[0]);
        String label = split[1];
        String category = split[2];
        long matches = Long.parseLong(split[3]);
        long time = Long.parseLong(split[4]);
        float meanDistance = Float.parseFloat(split[5]);
        float radius = Float.parseFloat(split[6]);
        String[] centerStrings = centerString.split(",");
        int dimensions = centerStrings.length;
        if (dimensions != 22) {
            throw new NumberFormatException("Minas model for KDD has 22 dimensions");
        }
        //
        float[] center = new float[dimensions];
        for (int i = 0; i < dimensions; i++) {
            center[i] = Float.parseFloat(centerStrings[i]);
        }
        return new Cluster(id, new Point(id, center, time), radius, label, category, matches, time);
    }

    public static Cluster apply(long id, Point center, float variance, String label) {
        return apply(id, center, variance, label, Cluster.CATEGORY_NORMAL);
    }

    public static Cluster apply(long id, Point center, float variance, String label, String category) {
        return apply(id, center, variance, label, category, 0);
    }

    public static Cluster apply(long id, Point center, float variance, String label, String category, long matches) {
        return apply(id, center, variance, label, category, matches, System.currentTimeMillis());
    }

    public static Cluster apply(long id, Point center, float variance, String label, String category, long matches, long time) {
        return new Cluster(id, center, variance, label, category, matches, time);
    }

    public JSONObject json() {
        return new JSONObject(this);
    }

    public long getId() {
        return id;
    }

    public void setId(long id) {
        this.id = id;
    }

    public Point getCenter() {
        return center;
    }

    public void setCenter(Point center) {
        this.center = center;
    }

    public float getVariance() {
        return variance;
    }

    public void setVariance(float variance) {
        this.variance = variance;
    }

    public String getLabel() {
        return label;
    }

    public void setLabel(String label) {
        this.label = label;
    }

    public String getCategory() {
        return category;
    }

    public void setCategory(String category) {
        this.category = category;
    }

    public long getMatches() {
        return matches;
    }

    public void setMatches(long matches) {
        this.matches = matches;
    }

    public long getTime() {
        return time;
    }

    public void setTime(long time) {
        this.time = time;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof Cluster)) return false;
        Cluster cluster = (Cluster) o;
        return getLabel().equals(cluster.getLabel()) && getCenter().equals(cluster.getCenter());
    }

    @Override
    public int hashCode() {
        return Objects.hash(getCenter(), getLabel());
    }

    public byte[] toBytes() {
        return SerializationUtils.serialize(this);
    }

    @Override
    public String toString() {
        return "Cluster{id=" + id + ", center=" + center + ", variance=" + variance + ", label='" + label + '\'' + ", category='" + category + '\'' + ", matches=" + matches + ", time=" + time + '}';
    }

    public void write(DataOutputStream out, Cluster cl) throws IOException {
        out.writeLong(cl.id);
        out.writeFloat(cl.variance);
        out.writeUTF(cl.label);
        out.writeUTF(cl.category);
        out.writeLong(cl.matches);
        out.writeLong(cl.time);
        cl.center.write(out, cl.center);
    }

    public Cluster read(DataInputStream in, Cluster cl) throws IOException {
        cl.id = in.readLong();
        cl.variance = in.readFloat();
        cl.label = in.readUTF();
        cl.category = in.readUTF();
        cl.matches = in.readLong();
        cl.time = in.readLong();
        if (cl.center == null) {
            cl.center = new Point();
        }
        cl.center = cl.center.read(in, cl.center);
        return cl;
    }

    @Override
    public void write(Kryo kryo, Output output, Cluster object) {
        output.writeLong(object.id);
        output.writeFloat(object.variance);
        output.writeString(object.label);
        output.writeString(object.category);
        output.writeLong(object.matches);
        output.writeLong(object.time);
        object.center.write(kryo, output, object.center);
    }

    @Override
    public Cluster read(Kryo kryo, Input input, Class<Cluster> type) {
        Cluster cl = new Cluster();
        cl.id = input.readLong();
        cl.variance = input.readFloat();
        cl.label = input.readString();
        cl.category = input.readString();
        cl.matches = input.readLong();
        cl.time = input.readLong();
        if (cl.center == null) {
            cl.center = new Point();
        }
        cl.center = cl.center.read(kryo, input, Point.class);
        return cl;
    }

}
