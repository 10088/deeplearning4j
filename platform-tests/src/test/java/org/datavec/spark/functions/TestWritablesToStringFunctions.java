/*
 *  ******************************************************************************
 *  *
 *  *
 *  * This program and the accompanying materials are made available under the
 *  * terms of the Apache License, Version 2.0 which is available at
 *  * https://www.apache.org/licenses/LICENSE-2.0.
 *  *
 *  *  See the NOTICE file distributed with this work for additional
 *  *  information regarding copyright ownership.
 *  * Unless required by applicable law or agreed to in writing, software
 *  * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *  * License for the specific language governing permissions and limitations
 *  * under the License.
 *  *
 *  * SPDX-License-Identifier: Apache-2.0
 *  *****************************************************************************
 */

package org.datavec.spark.functions;

import org.apache.spark.api.java.JavaPairRDD;
import org.apache.spark.api.java.JavaSparkContext;
import org.apache.spark.api.java.function.PairFunction;
import org.datavec.api.writable.DoubleWritable;
import org.datavec.api.writable.Text;
import org.datavec.api.writable.Writable;
import org.datavec.spark.BaseSparkTest;
import org.datavec.spark.transform.misc.SequenceWritablesToStringFunction;
import org.datavec.spark.transform.misc.WritablesToStringFunction;
import org.junit.jupiter.api.Tag;
import org.junit.jupiter.api.Test;
import org.nd4j.common.tests.tags.TagNames;
import scala.Tuple2;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import static org.junit.jupiter.api.Assertions.assertEquals;

@Tag(TagNames.FILE_IO)
@Tag(TagNames.JAVA_ONLY)
@Tag(TagNames.SPARK)
@Tag(TagNames.DIST_SYSTEMS)
public class TestWritablesToStringFunctions extends BaseSparkTest {

    @Test
    public void testCGroup() {
        List<Tuple2<String,String>> leftMap = new ArrayList<>();
        List<Tuple2<String,String>> rightMap = new ArrayList<>();

        leftMap.add(new Tuple2<>("cat","adam"));
        leftMap.add(new Tuple2<>("dog","adam"));

        rightMap.add(new Tuple2<>("fish","alex"));
        rightMap.add(new Tuple2<>("cat","alice"));
        rightMap.add(new Tuple2<>("dog","steve"));

        List<String> pets = Arrays.asList("cat","dog");



        JavaSparkContext sc = getContext();
        JavaPairRDD<String, String> left = sc.parallelize(leftMap).mapToPair((PairFunction<Tuple2<String, String>, String, String>) stringStringTuple2 -> stringStringTuple2);

        JavaPairRDD<String, String> right = sc.parallelize(rightMap).mapToPair((PairFunction<Tuple2<String, String>, String, String>) stringStringTuple2 -> stringStringTuple2);

        System.out.println(left.cogroup(right).collect());
    }

    @Test
    public void testWritablesToString() throws Exception {

        List<Writable> l = Arrays.asList(new DoubleWritable(1.5), new Text("someValue"));
        String expected = l.get(0).toString() + "," + l.get(1).toString();

        assertEquals(expected, new WritablesToStringFunction(",").call(l));
    }

    @Test
    public void testSequenceWritablesToString() throws Exception {

        List<List<Writable>> l = Arrays.asList(Arrays.asList(new DoubleWritable(1.5), new Text("someValue")),
                        Arrays.asList(new DoubleWritable(2.5), new Text("otherValue")));

        String expected = l.get(0).get(0).toString() + "," + l.get(0).get(1).toString() + "\n"
                        + l.get(1).get(0).toString() + "," + l.get(1).get(1).toString();

        assertEquals(expected, new SequenceWritablesToStringFunction(",").call(l));
    }
}
