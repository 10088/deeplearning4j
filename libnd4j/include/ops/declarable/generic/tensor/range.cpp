/* ******************************************************************************
 *
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License, Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0.
 *
 *  See the NOTICE file distributed with this work for additional
 *  information regarding copyright ownership.
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

//
// @author raver119@gmail.com
// @author Yurii Shyrma (iuriish@yahoo.com)
//

#include <system/op_boilerplate.h>
#if NOT_EXCLUDED(OP_range)

#include <ops/declarable/CustomOperations.h>
#include <ops/declarable/helpers/range.h>

namespace sd {
namespace ops {

CUSTOM_OP_IMPL(range, -2, 1, false, -2, -2) {
  auto output = OUTPUT_VARIABLE(0);

  const int numInArrs = block.width();
  const int numTArgs = block.getTArguments()->size();
  const int numIArgs = block.getIArguments()->size();

  NDArray *s = nullptr;
  NDArray *d = nullptr;

  bool localS = false;
  bool localD = false;
  // FIXME: this op should be fully moved to helpers

  if (output->isEmpty()) return sd::Status::OK;

  if (numInArrs > 0) {
    if (numInArrs == 1) {
      // limit = (*INPUT_VARIABLE(0))(0.);
      if (output->isR()) {
        s = NDArrayFactory::create_(0.0f, block.launchContext());
        d = NDArrayFactory::create_(1.0f, block.launchContext());
      } else {
        s = NDArrayFactory::create_(0, block.launchContext());
        d = NDArrayFactory::create_(1, block.launchContext());
      }
      localS = true;
      localD = true;
    } else if (numInArrs == 2) {
      s = INPUT_VARIABLE(0);
      // limit = (*INPUT_VARIABLE(1))(0.);
      if (output->isR()) {
        d = NDArrayFactory::create_(1.0f, block.launchContext());
      } else {
        d = NDArrayFactory::create_(1, block.launchContext());
      }
      localD = true;
    } else {
      s = INPUT_VARIABLE(0);
      // limit = (*INPUT_VARIABLE(1))(0.);
      d = INPUT_VARIABLE(2);
    }
  } else if (numIArgs > 0) {
    if (numIArgs == 1) {
      //  limit = INT_ARG(0);
    } else if (numIArgs == 2) {
      s = NDArrayFactory::create_(INT_ARG(0), block.launchContext());
      // limit = INT_ARG(1);
      d = NDArrayFactory::create_(1, block.launchContext());
    } else {
      s = NDArrayFactory::create_(INT_ARG(0), block.launchContext());
      // limit = INT_ARG(1);
      d = NDArrayFactory::create_(INT_ARG(2), block.launchContext());
    }

    localS = true;
    localD = true;
  } else if (numTArgs > 0) {
    if (numTArgs == 1) {
      // limit = T_ARG(0);
      s = NDArrayFactory::create_(0.0f, block.launchContext());
      d = NDArrayFactory::create_(1.0f, block.launchContext());
    } else if (numTArgs == 2) {
      s = NDArrayFactory::create_(T_ARG(0), block.launchContext());
      // limit = T_ARG(1);
      d = NDArrayFactory::create_(1.0f, block.launchContext());
    } else {
      s = NDArrayFactory::create_(T_ARG(0), block.launchContext());
      // limit = T_ARG(1);
      d = NDArrayFactory::create_(T_ARG(2), block.launchContext());
    }

    localS = true;
    localD = true;
  } else {
    REQUIRE_TRUE(
        false, 0,
        "CUSTOM RANGE OP: op should have inputs defined in any possible way: T_args, INT_args, or INPUT variables!");
  }

  helpers::range(block.launchContext(), *s, *d, *output);

  if (localS) delete s;

  if (localD) delete d;

  return sd::Status::OK;
}

DECLARE_SHAPE_FN(range) {
  const int numInArrs = block.width();
  const int numTArgs = block.getTArguments()->size();
  const int numIArgs = block.getIArguments()->size();

  sd::LongType steps = 0;
  sd::DataType dataType = block.numD() ? D_ARG(0) : sd::DataType::INHERIT;

  if (numInArrs > 0) {
    auto isR = INPUT_VARIABLE(0)->isR();
    auto isZ = INPUT_VARIABLE(0)->isZ();
    auto dtype = INPUT_VARIABLE(0)->dataType();

    if (isR) {
      double start(0), limit, delta(1);

      if (numInArrs == 1)
        limit = INPUT_VARIABLE(0)->e<double>(0);
      else if (numInArrs == 2) {
        start = INPUT_VARIABLE(0)->e<double>(0);
        limit = INPUT_VARIABLE(1)->e<double>(0);
      } else {
        start = INPUT_VARIABLE(0)->e<double>(0);
        limit = INPUT_VARIABLE(1)->e<double>(0);
        delta = INPUT_VARIABLE(2)->e<double>(0);
      }

      if (limit == start) {
        // Return [0] to match TF
        return SHAPELIST(ConstantShapeHelper::getInstance().vectorShapeInfo(0, dtype));
      }

      REQUIRE_TRUE(delta != 0, 0, "CUSTOM RANGE OP: delta should not be equal to zero !");

      steps = static_cast<sd::LongType>((limit - start) / delta);

      if (!block.numD()) dataType = INPUT_VARIABLE(0)->dataType();

      if (math::sd_abs<double>(start + steps * delta) < math::sd_abs<double>(limit)) ++steps;
    } else if (isZ) {
      sd::LongType start(0), limit, delta(1);

      if (numInArrs == 1)
        limit = INPUT_VARIABLE(0)->e<sd::LongType>(0);
      else if (numInArrs == 2) {
        start = INPUT_VARIABLE(0)->e<sd::LongType>(0);
        limit = INPUT_VARIABLE(1)->e<sd::LongType>(0);
      } else {
        start = INPUT_VARIABLE(0)->e<sd::LongType>(0);
        limit = INPUT_VARIABLE(1)->e<sd::LongType>(0);
        delta = INPUT_VARIABLE(2)->e<sd::LongType>(0);
      }

      if (limit == start) {
        // Return [0] to match TF
        return SHAPELIST(ConstantShapeHelper::getInstance().vectorShapeInfo(0, dtype));
      }

      REQUIRE_TRUE(delta != 0, 0, "CUSTOM RANGE OP: delta should not be equal to zero !");

      steps = static_cast<sd::LongType>((limit - start) / delta);

      if (!block.numD()) dataType = INPUT_VARIABLE(0)->dataType();

      if (math::sd_abs<double>(start + steps * delta) < math::sd_abs<double>(limit)) ++steps;
    }
  } else if (numIArgs > 0) {
    sd::LongType start(0), limit, delta(1);

    if (numIArgs == 1)
      limit = INT_ARG(0);
    else if (numIArgs == 2) {
      start = INT_ARG(0);
      limit = INT_ARG(1);
    } else {
      start = INT_ARG(0);
      limit = INT_ARG(1);
      delta = INT_ARG(2);
    }

    if (limit == start) {
      // Return [0] to match TF
      return SHAPELIST(ConstantShapeHelper::getInstance().vectorShapeInfo(0, sd::DataType::INT32));
    }

    REQUIRE_TRUE(delta != 0, 0, "CUSTOM RANGE OP: delta should not be equal to zero !");

    if (!block.numD()) {
      if (limit > DataTypeUtils::max<int>())
        dataType = sd::DataType::INT64;
      else
        dataType = sd::DataType::INT32;
    }

    steps = (limit - start) / delta;

    if (math::sd_abs<sd::LongType>(start + steps * delta) < math::sd_abs<sd::LongType>(limit)) ++steps;
  } else if (numTArgs > 0) {
    double start(0), limit, delta(1);

    if (numTArgs == 1)
      limit = T_ARG(0);
    else if (numTArgs == 2) {
      start = T_ARG(0);
      limit = T_ARG(1);
    } else {
      start = T_ARG(0);
      limit = T_ARG(1);
      delta = T_ARG(2);
    }

    if (limit == start) {
      // Return [0] to match TF
      return SHAPELIST(
          ConstantShapeHelper::getInstance().vectorShapeInfo(0, Environment::getInstance().defaultFloatDataType()));
    }

    REQUIRE_TRUE(delta != 0, 0, "CUSTOM RANGE OP: delta should not be equal to zero !");

    steps = static_cast<sd::LongType>((limit - start) / delta);

    if (!block.numD()) {
      if (Environment::getInstance().precisionBoostAllowed())
        dataType = sd::DataType::DOUBLE;
      else
        dataType = Environment::getInstance().defaultFloatDataType();
    }

    if (math::sd_abs<double>(start + steps * delta) < math::sd_abs<double>(limit)) ++steps;
  } else {
    REQUIRE_TRUE(
        false, 0,
        "CUSTOM RANGE OP: op should have inputs defined in any possible way: T_args, INT_args, or INPUT variables!");
  }

  REQUIRE_TRUE(steps > 0, 0, "CUSTOM RANGE OP: value of (limit-start)/delta should be positive !");

  return SHAPELIST(ConstantShapeHelper::getInstance().vectorShapeInfo(steps, dataType));
}

DECLARE_TYPES(range) {
  getOpDescriptor()->setAllowedInputTypes(sd::DataType::ANY)->setAllowedOutputTypes({ALL_FLOATS, ALL_INTS});
}
}  // namespace ops
}  // namespace sd

#endif
