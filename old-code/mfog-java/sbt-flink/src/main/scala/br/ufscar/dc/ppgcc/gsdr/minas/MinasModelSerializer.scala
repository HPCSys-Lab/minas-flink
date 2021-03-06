package br.ufscar.dc.ppgcc.gsdr.minas

import br.ufscar.dc.ppgcc.gsdr.minas.kmeans.{Cluster, Point}
import org.apache.flink.api.common.typeutils.{TypeSerializer, TypeSerializerSnapshot}
import org.apache.flink.api.scala.typeutils.{ CaseClassSerializer, TraversableSerializer, UnitSerializer }
import org.apache.flink.core.memory.{DataInputView, DataOutputView}
import scala.collection.JavaConversions
import scala.Function1

object MinasModelSerializer {
  val scalaFieldSerializers: Array[TypeSerializer[_]] = List(
    /* model: Vector[Cluster] */ TraversableSerializer.asInstanceOf[TypeSerializer[_]],
    /* sleep: Vector[Cluster] */ TraversableSerializer.asInstanceOf[TypeSerializer[_]],
    /* noMatch: Vector[Point] */ TraversableSerializer.asInstanceOf[TypeSerializer[_]],
    /* config: Map[String, Int] */ TraversableSerializer.asInstanceOf[TypeSerializer[_]],
    /* afterConsumedHook */ UnitSerializer.asInstanceOf[TypeSerializer[_]]
  ).toArray
}
class MinasModelSerializer extends CaseClassSerializer[MinasModel](MinasModel.getClass.asInstanceOf[Class[MinasModel]], MinasModelSerializer.scalaFieldSerializers) {
  override def snapshotConfiguration(): TypeSerializerSnapshot[MinasModel] = ???

  override def createInstance(fields: Array[AnyRef]): MinasModel = {
    type afterConsumedHookType = ((Option[String], Point, Cluster, Double)) => Unit
    MinasModel(
      model = fields(0).asInstanceOf[Vector[Cluster]],
      sleep = fields(0).asInstanceOf[Vector[Cluster]],
      noMatch = fields(0).asInstanceOf[Vector[Point]],
      config = fields(0).asInstanceOf[Map[String, Int]]
    )
  }
}
