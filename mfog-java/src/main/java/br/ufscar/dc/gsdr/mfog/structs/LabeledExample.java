package br.ufscar.dc.gsdr.mfog.structs;

import org.apache.commons.lang3.SerializationUtils;
import org.json.*;

import java.io.InputStream;
import java.io.Serializable;
import java.util.Objects;

public class LabeledExample implements Serializable {

    public static LabeledExample fromJson(String src) {
        return fromJson(new JSONObject(src));
    }
    public static LabeledExample fromJson(JSONObject src) {
        LabeledExample l = new LabeledExample();
        l.label = src.getString("label");
        l.point = Point.fromJson(src.getJSONObject("point"));
        return l;
    }
    public static LabeledExample fromBytes(byte[] bytes) {
        return SerializationUtils.deserialize(bytes);
    }
    public static LabeledExample fromBytes(InputStream stream) {
        return SerializationUtils.deserialize(stream);
    }


    static public LabeledExample fromKyotoCSV(int id, String line) {
        // 0.0,0.0,0.0,0.0,0.0,0.0,0.4,0.0,0.0,0.0,0.0,0.0,1,0,0,0,0,0,0,0,1,0,N
        String[] lineSplit = line.split(",");
        float[] floats = new float[lineSplit.length -1];
        for (int j = 0; j < floats.length -1; j++) {
            floats[j] = Float.parseFloat(lineSplit[j]);
        }
        String label = lineSplit[lineSplit.length - 1];
        Point point = Point.apply(id, floats);
        return new LabeledExample(label, point);
    }

    public Point point;
    public String label;

    public LabeledExample() {}
    public LabeledExample(String label, Point point) {
        this.point = point;
        this.label = label;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof LabeledExample)) return false;
        LabeledExample that = (LabeledExample) o;
        return point.equals(that.point) && label.equals(that.label);
    }

    @Override
    public int hashCode() {
        return Objects.hash(point, label);
    }

    public Point getPoint() {
        return point;
    }

    public void setPoint(Point point) {
        this.point = point;
    }

    public String getLabel() {
        return label;
    }

    public void setLabel(String label) {
        this.label = label;
    }

    @Override
    public String toString() {
        return "LabeledExample{point=" + point + ", label='" + label + "'}";
    }

    public JSONObject json() {
        return new JSONObject(this);
    }

    public byte[] toBytes() {
        return SerializationUtils.serialize(this);
    }
}